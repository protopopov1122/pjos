#include "sat_solver/Format.h"
#include <iostream>

namespace sat_solver {

    namespace internal {
        std::ostream &operator<<(std::ostream &os, const LiteralFormatter &literal) {
            return os << literal.literal.Get();
        }

        std::ostream &operator<<(std::ostream &os, const ClauseViewFormatter &clause) {
            for (auto lit : clause.clause) {
                os << Format(lit) << ' ';
            }
            os << Literal::Terminator;
            return os;
        }

        std::ostream &operator<<(std::ostream &os, const FormulaFormatter &state) {
            os << "p cnf " << state.formula.NumOfVariables() << " " << state.formula.NumOfClauses() << std::endl;
            for (auto it = state.formula.begin(); it != state.formula.end(); it = std::next(it)) {
                os << Format(*it);
                if (std::next(it) != state.formula.end()) {
                    os << std::endl;
                }
            }
            return os;
        }

        std::ostream &operator<<(std::ostream &os, const AssignmentFormatter &assn) {
            for (auto it = assn.assignment.begin(); it != assn.assignment.end(); it = std::next(it)) {
                auto [variable, assignment] = *it;
                if (assignment != VariableAssignment::Unassigned) {
                    os << (assignment == VariableAssignment::False ? -1 : 1) * variable;
                    if (std::next(it) != assn.assignment.end()) {
                        os << ' ';
                    }
                }
            }
            return os;
        }

        std::ostream &operator<<(std::ostream &os, const SolverStatusFormatter &status) {
            switch (status.status) {
                case SolverStatus::Satisfied:
                    os << "SATISFIABLE";
                    break;

                case SolverStatus::Unsatisfied:
                    os << "UNSATISFIABLE";
                    break;

                case SolverStatus::Unknown:
                    os << "UNKNOWN";
                    break;

                case SolverStatus::Solving:
                    os << "SOLVING";
                    break;
            }
            return os;
        }
    }

    internal::LiteralFormatter Format(Literal literal) {
        return {literal};
    }

    internal::ClauseViewFormatter Format(const ClauseView &clause) {
        return {clause};
    }

    internal::FormulaFormatter Format(const Formula &state) {
        return {state};
    }

    internal::AssignmentFormatter Format(const Assignment &assn) {
        return {assn};
    }

    internal::SolverStatusFormatter Format(SolverStatus status) {
        return {status};
    }
}
