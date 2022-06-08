/*
    SPDX-License-Identifier: MIT
    SPDX-FileCopyrightText: 2022 Protopopovs Jevgenijs
*/

#include "pjos/DpllSolver.h"
#include "pjos/Format.h"

// This file contains simple implementation of DPLL solver.
// The solver is not practical by itself, however may be used as a baseline
// for debugging issues.

namespace pjos {

    DpllSolver::DpllSolver(const Formula &formula)
        : BaseSolver::BaseSolver{formula} {}

    const std::string &DpllSolver::Signature() {
        static std::string sig{PJOS_IDENTIFIER " (DPLL) " PJOS_VERSION};
        return sig;
    }

    SolverStatus DpllSolver::SolveImpl() {
        auto pending_assignments_iter = this->pending_assignments.begin();
        for (;;) {
            if (this->interrupt_requested.load() || (this->interrupt_request_fn != nullptr && this->interrupt_request_fn())) {
                return SolverStatus::Unknown;
            }

            auto [bcp_result, conflict_clause] = this->UnitPropagation();
            if (bcp_result == UnitPropagationResult::Sat) { // BCP satisfied the formula
                return SolverStatus::Satisfied;
            } else if (bcp_result == UnitPropagationResult::Unsat) { // Conflict was encountered during BCP
                Literal::UInt variable;
                VariableAssignment assignment;
                bool undo_decision = true;
                while (undo_decision) { // Undo the last decision if possible
                    auto trail_entry = this->trail.Top();
                    if (trail_entry == nullptr) { // There is no decision to undo, return UNSAT
                        return SolverStatus::Unsatisfied;
                    }

                    variable = trail_entry->variable;
                    if (DecisionTrail::IsPropagatedFromClause(trail_entry->reason) || trail_entry->reason == DecisionTrail::ReasonPropagation) { // Undo propagation
                        this->Assign(variable, VariableAssignment::Unassigned);
                    } else { // Remember the last decision
                        undo_decision = false;
                        assignment = trail_entry->assignment;
                    }
                    this->trail.Pop();
                }

                // Flip the last decision and proceed
                auto new_assignment = FlipVariableAssignment(assignment);
                this->trail.Propagation(variable, new_assignment);
                this->Assign(variable, new_assignment);
            } else if (pending_assignments_iter == this->pending_assignments.end()) { // There are no pending assignments.
                                                                                      // Find an unassigned variable and assign it to true
#ifndef NDEBUG
                bool made_decision = false;
#endif
                for (std::int64_t variable = this->assignment.NumOfVariables(); variable > 0; variable--) {
                    auto assn = this->assignment[variable];
                    if (assn == VariableAssignment::Unassigned) {
                        this->trail.Decision(variable, VariableAssignment::True);
                        this->Assign(variable, VariableAssignment::True);
#ifndef NDEBUG
                        made_decision = true;
#endif
                        break;
                    }
                }
                assert(made_decision);
            } else { // There is pending assignment, perform it
                auto [variable, variable_assignment, is_assumption] = *pending_assignments_iter;
                std::advance(pending_assignments_iter, 1);
                if (!this->PerformPendingAssignment(variable, variable_assignment, is_assumption)) {
                    return SolverStatus::Unsatisfied;
                }
            }
        }
    }

    ModifiableDpllSolver::ModifiableDpllSolver()
        : ModifiableDpllSolver::ModifiableDpllSolver(Formula{}) {}

    ModifiableDpllSolver::ModifiableDpllSolver(Formula formula)
        : ModifiableSolverBase::ModifiableSolverBase{std::move(formula)},
          DpllSolver::DpllSolver{this->owned_formula} {}
}
