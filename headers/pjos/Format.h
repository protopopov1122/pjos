/*
    SPDX-License-Identifier: MIT
    SPDX-FileCopyrightText: 2022 Protopopovs Jevgenijs
*/

#ifndef PJOS_FORMAT_H_
#define PJOS_FORMAT_H_

#include "pjos/Core.h"
#include "pjos/Literal.h"
#include "pjos/Clause.h"
#include "pjos/Formula.h"
#include "pjos/Assignment.h"
#include "pjos/BaseSolver.h"
#include <iostream>
#include <type_traits>

// Output formatting functions for solver classes.
// These are isolated from the main class code to avoid
// mixing up internal data management and pretty-printing code.
// Additional level of indirectness (Format functions instead
// of direct operator<< overloads) is used to permit more flexible
// parameterized formatting.

namespace pjos {

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
            const BaseSolver<T> &solver;
            bool include_model;
        };

        struct StringFormatter {
            std::string_view str;
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

            if (fmt.solver.Status() == SolverStatus::Satisfied && fmt.include_model) {
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
    internal::StringFormatter Format(std::string_view);

    template <typename T>
    internal::SolverFormatter<T> Format(const BaseSolver<T> &solver, bool include_model = true) {
        return {solver, include_model};
    }
}

#endif
