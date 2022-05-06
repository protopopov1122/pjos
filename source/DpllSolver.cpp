#include "sat_solver/Solver.h"
#include "sat_solver/Format.h"
#include <iostream>

namespace sat_solver {

    internal::SolverState::SolverState(const Formula &formula)
        : formula{formula}, assignment{static_cast<std::size_t>(formula.NumOfVariables())}, trail{assignment, [this](auto var, auto assn) {this->Assign(var, assn);}} {
        
        this->Rebuild();
    }

    void internal::SolverState::Rebuild() {
        this->assignment.Reset();
        this->watchers.clear();
        this->variable_index.clear();

        for (std::size_t clause_idx = 0; clause_idx < this->formula.NumOfClauses(); clause_idx++) {
            const auto &clause = this->formula[clause_idx];
            this->watchers.emplace_back(clause);

            for (auto literal : clause) {
                auto index_it = this->variable_index.find(literal.Variable());
                if (index_it == this->variable_index.end()) {
                    index_it = this->variable_index.emplace(std::make_pair(literal.Variable(), VariableIndexEntry{})).first;
                }

                auto &index_entry = index_it->second;
                if (literal.Polarity() == LiteralPolarity::Positive) {
                    index_entry.positive_clauses.emplace_back(clause_idx);
                } else {
                    index_entry.negative_clauses.emplace_back(clause_idx);
                }
            }
        }
    }

    void internal::SolverState::UpdateWatchers(Literal::Int variable) {
        for (auto affected_watcher : this->variable_index[variable].positive_clauses) {
            this->watchers[affected_watcher].Update(this->assignment);
        }
        for (auto affected_watcher : this->variable_index[variable].negative_clauses) {
            this->watchers[affected_watcher].Update(this->assignment);
        }
    }

    void internal::SolverState::Assign(Literal::Int variable, VariableAssignment assignment) {
        this->assignment.Set(variable, assignment);
        this->UpdateWatchers(variable);
    }

    DpllSolver::DpllSolver(const Formula &formula)
        : state{formula} {}

    SolverStatus DpllSolver::Solve() {
        for (;;) {
            auto bcp_result = this->Bcp();
            if (bcp_result == BcpResult::Sat) {
                return SolverStatus::Satisfiable;
            } else if (bcp_result == BcpResult::Unsat) {
                auto [variable, assignment] = this->state.trail.UndoDecision();
                if (variable == Literal::Terminator) {
                    return SolverStatus::Unsatisfiable;
                }
                this->state.trail.Propagation(variable, FlipVariableAssignment(assignment));
            } else {
                for (std::int64_t variable = this->state.assignment.NumOfVariables() - 1; variable >= 0; variable--) {
                    auto assn = this->state.assignment.Of(variable);
                    if (assn == VariableAssignment::Unassigned) {
                        this->state.trail.Decision(variable, VariableAssignment::True);
                        break;
                    }
                }
            }
        }
    }

    DpllSolver::BcpResult DpllSolver::Bcp() {
        bool all_satisfied = false, propagate = true;
        while (propagate && !all_satisfied) {
            propagate = false;
            all_satisfied = true;
            for (std::size_t i = 0; !propagate && i < this->state.watchers.size(); i++) {
                auto &watcher = this->state.watchers[i];

                auto watcher_status = watcher.Status();
                auto watcher_unit = watcher.GetWatchedLiteralIndices().first;

                all_satisfied = all_satisfied && watcher_status == ClauseStatus::Satisfied;
                if (watcher_status == ClauseStatus::Unit) {
                    auto [variable, assignment] = this->state.formula[i][watcher_unit].Assignment();

                    this->state.trail.Propagation(variable, assignment);
                    propagate = true;
                } else if (watcher_status == ClauseStatus::Unsatisfied) {
                    return BcpResult::Unsat;
                }
            }
        }

        return all_satisfied
            ? BcpResult::Sat
            : BcpResult::Pass;
    }
}
