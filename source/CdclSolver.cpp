#include "sat_solver/Solver.h"
#include <cassert>

namespace sat_solver {

    CdclSolver::CdclSolver(Formula formula)
        : ModifiableSolverBase::ModifiableSolverBase(*this, std::move(formula)),
          BaseSolver::BaseSolver{this->owned_formula},
          analysis_track(formula.NumOfVariables(), AnalysisTrackState::Untracked) {}

    SolverStatus CdclSolver::Solve() {
        for (;;) {
            auto [bcp_result, conflict_clause] = this->UnitPropagation();
            if (bcp_result == UnitPropagationResult::Sat) {
                return SolverStatus::Satisfied;
            } else if (bcp_result == UnitPropagationResult::Unsat) {
                if (this->trail.Level() == 0) {
                    return SolverStatus::Unsatisfied;
                }

                auto learned_clause = this->AnalyzeConflict(this->formula[conflict_clause]);
                this->AppendClause(std::move(learned_clause));
                
                // TODO: Non-chronological backjump
                bool undo_decision = true;
                while (undo_decision) {
                    auto trail_entry = this->trail.Undo();
                    if (!trail_entry.has_value()) {
                        return SolverStatus::Unsatisfied;
                    }

                    this->Assign(trail_entry->variable, VariableAssignment::Unassigned);
                    undo_decision = trail_entry->reason != DecisionTrail::ReasonDecision;
                }
            } else {
                for (std::int64_t variable = this->assignment.NumOfVariables(); variable > 0; variable--) {
                    auto assn = this->assignment[variable];
                    if (assn == VariableAssignment::Unassigned) {
                        this->trail.Decision(variable, VariableAssignment::True);
                        this->Assign(variable, VariableAssignment::True);
                        break;
                    }
                }
            }
        }
    }

    Clause CdclSolver::AnalyzeConflict(const ClauseView &conflict) {
        assert(this->trail.Level() > 0);
        std::fill(this->analysis_track.begin(), this->analysis_track.end(), AnalysisTrackState::Untracked);

        ClauseBuilder learned_clause;

        const ClauseView *clause = std::addressof(conflict);
        std::size_t trail_index = this->trail.Length() - 1;
        std::size_t number_of_paths = 1;
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
        learned_clause.Add(Literal{this->trail[trail_index].variable, FlipVariableAssignment(this->trail[trail_index].assignment)});

        return learned_clause.Make();
    }

    BaseSolver::AnalysisTrackState &CdclSolver::AnalysisTrackOf(Literal::Int variable) {
        return this->analysis_track[variable - 1];
    }
}