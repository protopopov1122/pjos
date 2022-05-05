#ifndef SAT_SOLVER_FORMAT_H_
#define SAT_SOLVER_FORMAT_H_

#include "sat_solver/Literal.h"
#include "sat_solver/Clause.h"
#include "sat_solver/Formula.h"
#include <iosfwd>

namespace sat_solver {

    namespace internal {
        
        struct LiteralFormatter {
            Literal literal;
        };

        struct ClauseViewFormatter {
            const ClauseView &clause;
        };

        struct FormulaFormatter {
            const Formula &state;
        };

        std::ostream &operator<<(std::ostream &, const LiteralFormatter &);
        std::ostream &operator<<(std::ostream &, const ClauseViewFormatter &);
        std::ostream &operator<<(std::ostream &, const FormulaFormatter &);
    };

    internal::LiteralFormatter Format(Literal);
    internal::ClauseViewFormatter Format(const ClauseView &);
    internal::FormulaFormatter Format(const Formula &);
}

#endif
