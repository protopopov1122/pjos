#include "sat_solver/Solver.h"
#include <algorithm>
#include <cassert>
#include <iostream>

namespace sat_solver {

    CdclSolver::CdclSolver(Formula formula)
        : ModifiableSolverBase::ModifiableSolverBase(*this, std::move(formula)),
          BaseSolver::BaseSolver{this->owned_formula},
          analysis_track(formula.NumOfVariables(), AnalysisTrackState::Untracked),
          vsids{this->formula, this->assignment} {
        
        this->assignment_callback = [this](auto variable, auto assignment) {
            this->OnAssign(variable, assignment);
        };
    }

    SolverStatus CdclSolver::Solve() {
        this->AssignPureLiterals();
        for (;;) {
            auto [bcp_result, conflict_clause] = this->UnitPropagation();
            if (bcp_result == UnitPropagationResult::Sat) {
                return SolverStatus::Satisfied;
            } else if (bcp_result == UnitPropagationResult::Unsat) {
                if (this->trail.Level() == 0) {
                    return SolverStatus::Unsatisfied;
                }

                auto [learned_clause, backjump_level] = this->AnalyzeConflict(this->formula[conflict_clause]);
                this->AppendClause(std::move(learned_clause));
                
                if (!this->Backjump(backjump_level)) {
                    return SolverStatus::Unsatisfied;
                }
            } else {
                auto variable = this->vsids.SuggestedVariableForAssignment();
                assert(variable != Literal::Terminator);
                const auto &variable_index_entry = this->variable_index[variable - 1];
                auto variable_assignment = variable_index_entry.positive_clauses.size() >= variable_index_entry.negative_clauses.size()
                    ? VariableAssignment::True
                    : VariableAssignment::False;
                this->trail.Decision(variable, variable_assignment);
                this->Assign(variable, variable_assignment);
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
                    this->vsids.OnVariableActivity(trail_entry->variable);
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
        this->vsids.OnVariableActivity(trail_entry.variable);
        assert(trail_entry.level == this->trail.Level());
        assert(backjump_level < this->trail.Level());

        return std::make_pair(learned_clause.Make(), backjump_level);
    }

    BaseSolver::AnalysisTrackState &CdclSolver::AnalysisTrackOf(Literal::Int variable) {
        return this->analysis_track[variable - 1];
    }

    bool CdclSolver::Backjump(std::size_t level) {
        while (this->trail.Level() > level) {
            auto trail_entry = this->trail.Undo();
            if (!trail_entry.has_value()) {
                return false;
            }

            this->Assign(trail_entry->variable, VariableAssignment::Unassigned);
        }
        return true;
    }

    void CdclSolver::AttachClause(std::size_t clause_index, const ClauseView &clause) {
        this->BaseSolver::AttachClause(clause_index, clause);

        if (static_cast<std::size_t>(this->formula.NumOfVariables()) > this->analysis_track.size()) {
            this->analysis_track.insert(this->analysis_track.end(), this->formula.NumOfVariables() - this->analysis_track.size(), AnalysisTrackState::Untracked);
        }
        this->vsids.OnUpdate();
    }

    void CdclSolver::OnAssign(Literal::Int variable, VariableAssignment) {
        this->vsids.OnVariableAssignment(variable);
    }
}