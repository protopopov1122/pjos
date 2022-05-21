#include "sat_solver/Solver.h"
#include <algorithm>
#include <cassert>

namespace sat_solver {

    std::size_t CdclSolver::VariableOccurences::operator()(Literal::UInt variable) const {
        const auto &index_entry = this->solver.VariableIndex(variable);
        return index_entry.positive_clauses.size() + index_entry.negative_clauses.size();
    }

    CdclSolver::CdclSolver()
        : CdclSolver::CdclSolver(Formula{}) {}

    CdclSolver::CdclSolver(Formula formula)
        : ModifiableSolverBase::ModifiableSolverBase(std::move(formula)),
          BaseSolver::BaseSolver{this->owned_formula},
          analysis_track(formula.NumOfVariables(), AnalysisTrackState::Untracked),
          vsids{this->formula, this->assignment, VariableOccurences{*this}} {}

    const std::string &CdclSolver::Signature() {
        static std::string sig{"SAT Solver (CDCL)"};
        return sig;
    }

    SolverStatus CdclSolver::SolveImpl() {
        bool analyze_final_conflict = false;
        std::swap(analyze_final_conflict, this->analyze_final_conflict);
        this->ScanPureLiterals();

        auto pending_assignments_iter = this->pending_assignments.begin();
        std::size_t number_of_assumptions{0};
        for (;;) {
            if (this->interrupt_requested.load()) {
                return SolverStatus::Unknown;
            }

            auto [bcp_result, conflict_clause] = this->UnitPropagation();
            if (bcp_result == UnitPropagationResult::Sat) {
                Literal conflict[2];
                if (this->VerifyPendingAssignments(pending_assignments_iter, this->pending_assignments.end(), conflict[0])) {
                    return SolverStatus::Satisfied;
                } else {
                    if (analyze_final_conflict) {
                        this->AnalyzeFinalConflict(&conflict[0], &conflict[1], true);
                    }
                    return SolverStatus::Unsatisfied;
                }
            } else if (bcp_result == UnitPropagationResult::Unsat) {
                if (this->trail.Level() == 0) {
                    if (analyze_final_conflict) {
                        const auto &conflict = this->formula[conflict_clause];
                        this->AnalyzeFinalConflict(conflict.begin(), conflict.end(), false);
                    }
                    return SolverStatus::Unsatisfied;
                }

                auto [learned_clause, backjump_level] = this->AnalyzeConflict(this->formula[conflict_clause]);
                this->AppendClause(std::move(learned_clause));
                
                if (backjump_level < number_of_assumptions || !this->Backjump(backjump_level)) {
                    if (analyze_final_conflict) {
                        const auto &conflict = this->formula[conflict_clause];
                        this->AnalyzeFinalConflict(conflict.begin(), conflict.end(), false);
                    }
                    return SolverStatus::Unsatisfied;
                }
            } else if (pending_assignments_iter == this->pending_assignments.end()) {
                auto variable = this->vsids.TopVariable();
                assert(variable != Literal::Terminator);
                const auto &variable_index_entry = this->VariableIndex(variable);
                auto variable_assignment = variable_index_entry.positive_clauses.size() >= variable_index_entry.negative_clauses.size()
                    ? VariableAssignment::True
                    : VariableAssignment::False;
                this->trail.Decision(variable, variable_assignment);
                this->Assign(variable, variable_assignment);
            } else {
                auto [variable, variable_assignment, is_assumption] = *pending_assignments_iter;
                std::advance(pending_assignments_iter, 1);
                if (!this->PerformPendingAssignment(variable, variable_assignment, is_assumption)) {
                    if (analyze_final_conflict) {
                        Literal conflict[2] = {Literal{variable, variable_assignment}};
                        this->AnalyzeFinalConflict(&conflict[0], &conflict[1], true);
                    }
                    return SolverStatus::Unsatisfied;
                }
                if (is_assumption) {
                    number_of_assumptions++;
                }
            }
        }
    }

    std::pair<Clause, std::size_t> CdclSolver::AnalyzeConflict(const ClauseView &conflict) {
        assert(this->trail.Level() > 0);
        std::fill(this->analysis_track.begin(), this->analysis_track.end(), AnalysisTrackState::Untracked);

        ClauseBuilder learned_clause;

        const ClauseView *clause = std::addressof(conflict);
        std::size_t trail_index = this->trail.Length() - 1;
        std::size_t number_of_paths = 1;
        std::size_t backjump_level = 0;
        do {
            for (auto literal : *clause) {
                auto variable = literal.Variable();
                if (this->AnalysisTrackOf(variable) != AnalysisTrackState::Untracked) {
                    continue;
                }

                auto trail_entry = this->trail.Find(variable);
                assert(trail_entry != nullptr);
                if (trail_entry->level >= this->trail.Level()) {
                    this->AnalysisTrackOf(variable) = AnalysisTrackState::Pending;
                    number_of_paths++;
                } else {
                    learned_clause.Add(Literal{variable, FlipVariableAssignment(trail_entry->assignment)});
                    backjump_level = std::max(backjump_level, trail_entry->level);
                    this->vsids.VariableActive(trail_entry->variable);
                }
            }
            number_of_paths--;

            while (this->AnalysisTrackOf(this->trail[trail_index].variable) != AnalysisTrackState::Pending) {
                assert(trail_index > 0);
                trail_index--;
            }
            this->AnalysisTrackOf(this->trail[trail_index].variable) = AnalysisTrackState::Processed;
            const auto &trail_entry = this->trail[trail_index];
            if (DecisionTrail::IsPropagatedFromClause(trail_entry.reason)) {
                clause = std::addressof(this->formula[trail_entry.reason]);
            } else {
                assert(number_of_paths == 1);
            }
        } while (number_of_paths > 1);

        const auto &trail_entry = this->trail[trail_index];
        learned_clause.Add(Literal{trail_entry.variable, FlipVariableAssignment(trail_entry.assignment)});
        this->vsids.VariableActive(trail_entry.variable);
        assert(trail_entry.level > 0);
        if (backjump_level == 0) {
            backjump_level = std::max(backjump_level, trail_entry.level - 1);
        }
        assert(trail_entry.level == this->trail.Level());
        assert(backjump_level < this->trail.Level());

        return std::make_pair(learned_clause.Make(), backjump_level);
    }

    BaseSolver<CdclSolver>::AnalysisTrackState &CdclSolver::AnalysisTrackOf(Literal::UInt variable) {
        return this->analysis_track[variable - 1];
    }

    bool CdclSolver::Backjump(std::size_t level) {
        while (this->trail.Level() > level) {
            auto trail_entry = this->trail.Top();
            if (trail_entry == nullptr) {
                return false;
            }

            assert(trail_entry->reason != DecisionTrail::ReasonAssumption);

            this->Assign(trail_entry->variable, VariableAssignment::Unassigned);
            this->trail.Pop();
        }
        return true;
    }

    void CdclSolver::AttachClause(std::size_t clause_index, const ClauseView &clause) {
        this->BaseSolver<CdclSolver>::AttachClause(clause_index, clause);

        if (this->formula.NumOfVariables() > this->analysis_track.size()) {
            this->analysis_track.insert(this->analysis_track.end(), this->formula.NumOfVariables() - this->analysis_track.size(), AnalysisTrackState::Untracked);
        }
        this->vsids.FormulaUpdated();
    }

    void CdclSolver::DetachClause(std::size_t clause_index, const ClauseView &clause) {
        this->BaseSolver<CdclSolver>::DetachClause(clause_index, clause);

        if (this->formula.NumOfVariables() < this->analysis_track.size()) {
            this->analysis_track.erase(this->analysis_track.begin() + this->formula.NumOfVariables(), this->analysis_track.end());
        }
        this->vsids.FormulaUpdated();
    }

    void CdclSolver::OnVariableAssignment(Literal::UInt variable, VariableAssignment) {
        this->vsids.VariableAssigned(variable);
    }
}