#include "sat_solver/Solver.h"
#include "sat_solver/Format.h"
#include <iostream>

namespace sat_solver {

    DpllSolver::DpllSolver(const Formula &formula)
        : BaseSolver::BaseSolver{formula} {}

    SolverStatus DpllSolver::SolveImpl() {
        auto pending_assignments_iter = this->pending_assignments.begin();
        for (;;) {
            auto [bcp_result, conflict_clause] = this->UnitPropagation();
            if (bcp_result == UnitPropagationResult::Sat) {
                return SolverStatus::Satisfied;
            } else if (bcp_result == UnitPropagationResult::Unsat) {
                Literal::Int variable;
                VariableAssignment assignment;
                bool undo_decision = true;
                while (undo_decision) {
                    auto trail_entry = this->trail.Top();
                    if (trail_entry == nullptr) {
                        return SolverStatus::Unsatisfied;
                    }

                    variable = trail_entry->variable;
                    if (DecisionTrail::IsPropagatedFromClause(trail_entry->reason) || trail_entry->reason == DecisionTrail::ReasonPropagation) {
                        this->Assign(variable, VariableAssignment::Unassigned);
                    } else {
                        undo_decision = false;
                        assignment = trail_entry->assignment;
                    }
                    this->trail.Pop();
                }

                auto new_assignment = FlipVariableAssignment(assignment);
                this->trail.Propagation(variable, new_assignment);
                this->Assign(variable, new_assignment);
            } else if (pending_assignments_iter != this->pending_assignments.end()) {
                for (std::int64_t variable = this->assignment.NumOfVariables(); variable > 0; variable--) {
                    auto assn = this->assignment[variable];
                    if (assn == VariableAssignment::Unassigned) {
                        this->trail.Decision(variable, VariableAssignment::True);
                        this->Assign(variable, VariableAssignment::True);
                        break;
                    }
                }
            } else {
                auto [variable, variable_assignment, is_assumption] = *pending_assignments_iter;
                std::advance(pending_assignments_iter, 1);
                if (!this->PerformPendingAssignment(variable, variable_assignment, is_assumption)) {
                    return SolverStatus::Unsatisfied;
                }
            }
        }
    }

    ModifiableDpllSolver::ModifiableDpllSolver(Formula formula)
        : ModifiableSolverBase::ModifiableSolverBase{std::move(formula)},
          DpllSolver::DpllSolver{this->owned_formula} {}
}
