#include "sat_solver/Solver.h"
#include "sat_solver/Format.h"
#include <iostream>

namespace sat_solver {

    DpllSolver::DpllSolver(const Formula &formula)
        : BaseSolver::BaseSolver{formula} {}

    SolverStatus DpllSolver::Solve() {
        for (;;) {
            auto bcp_result = this->UnitPropagation();
            if (bcp_result == UnitPropagationResult::Sat) {
                return SolverStatus::Satisfied;
            } else if (bcp_result == UnitPropagationResult::Unsat) {
                Literal::Int variable;
                VariableAssignment assignment;
                bool undo_decision = true;
                while (undo_decision) {
                    auto clause_index = this->trail.Undo(variable, assignment);
                    if (variable == Literal::Terminator) {
                        return SolverStatus::Unsatisfied;
                    }
                    if (clause_index != DecisionTrail::DecisionLabel) {
                        this->Assign(variable, VariableAssignment::Unassigned);
                    } else {
                        undo_decision = false;
                    }
                }

                auto new_assignment = FlipVariableAssignment(assignment);
                this->trail.Propagation(variable, new_assignment);
                this->Assign(variable, new_assignment);
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
}
