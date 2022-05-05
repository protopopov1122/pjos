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
            os << "p cnf " << state.state.NumOfVariables() << " " << state.state.NumOfClauses() << std::endl;
            for (auto it = state.state.begin(); it != state.state.end(); it = std::next(it)) {
                os << Format(*it);
                if (std::next(it) != state.state.end()) {
                    os << std::endl;
                }
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
}
