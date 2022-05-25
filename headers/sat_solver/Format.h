#ifndef SAT_SOLVER_FORMAT_H_
#define SAT_SOLVER_FORMAT_H_

#include "sat_solver/Core.h"
#include "sat_solver/Literal.h"
#include "sat_solver/Clause.h"
#include "sat_solver/Formula.h"
#include "sat_solver/Assignment.h"
#include "sat_solver/BaseSolver.h"
#include <iostream>
#include <type_traits>

namespace sat_solver {

    namespace internal {
        
        struct LiteralFormatter {
            Literal literal;
        };

        struct ClauseViewFormatter {
            const ClauseView &clause;
        };

        struct FormulaFormatter {
            const Formula &formula;
        };

        struct AssignmentFormatter {
            const Assignment &assignment;
        };

        struct SolverStatusFormatter {
            SolverStatus status;
        };

        template <typename T>
        struct SolverFormatter {
            const T &solver;
        };

        struct StringFormatter {
            const std::string &str;
        };

        std::ostream &operator<<(std::ostream &, const LiteralFormatter &);
        std::ostream &operator<<(std::ostream &, const ClauseViewFormatter &);
        std::ostream &operator<<(std::ostream &, const FormulaFormatter &);
        std::ostream &operator<<(std::ostream &, const AssignmentFormatter &);
        std::ostream &operator<<(std::ostream &, const SolverStatusFormatter &);
        std::ostream &operator<<(std::ostream &, const StringFormatter &);

        template <typename T>
        std::ostream &operator<<(std::ostream &os, const SolverFormatter<T> &fmt) {
            os << "s " << Format(fmt.solver.Status());

            if (fmt.solver.Status() == SolverStatus::Satisfied) {
                os << std::endl << "v ";
                const auto &assn = fmt.solver.GetAssignment();
                for (Literal::UInt variable = 1; variable <= assn.NumOfVariables(); variable++) {
                    os << LiteralFormatter{Literal{variable, assn[variable]}} << ' ';
                }
                os << '0';
            }
            return os;
        }
    };

    internal::LiteralFormatter Format(Literal);
    internal::ClauseViewFormatter Format(const ClauseView &);
    internal::FormulaFormatter Format(const Formula &);
    internal::AssignmentFormatter Format(const Assignment &);
    internal::SolverStatusFormatter Format(SolverStatus);
    internal::StringFormatter Format(const std::string &);

    template <typename T>
    internal::SolverFormatter<T> Format(const T &solver) {
        static_assert(std::is_base_of_v<BaseSolver<T>, T>);
        return {solver};
    }
}

#endif
