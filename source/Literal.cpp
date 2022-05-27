/*
    SPDX-License-Identifier: MIT
    SPDX-FileCopyrightText: 2022 Protopopovs Jevgenijs
*/

#include "pjos/Literal.h"
#include "pjos/Error.h"
#include <cstdlib>

namespace pjos {

    Literal::Literal(Int literal)
        : literal{literal} {
        if (literal == Literal::Terminator) {
            throw SatError{"Literal cannot be zero"};
        }
    }

    Literal::Literal(UInt literal, VariableAssignment assn) {
        if (literal == Literal::Terminator) {
            throw SatError{"Literal cannot be zero"};
        }
        switch (assn) {
            case VariableAssignment::True:
            case VariableAssignment::Unassigned:
                this->literal = static_cast<Int>(literal);
                break;

            case VariableAssignment::False:
                this->literal = -1 * static_cast<Int>(literal);
                break;
        }
    }

    void swap(Literal &l1, Literal &l2) {
        std::swap(l1.literal, l2.literal);
    }
}
