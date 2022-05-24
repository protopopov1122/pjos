#include "sat_solver/Literal.h"
#include "sat_solver/Error.h"
#include <cstdlib>

namespace sat_solver {

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
