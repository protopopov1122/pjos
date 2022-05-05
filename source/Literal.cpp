#include "sat_solver/Literal.h"
#include "sat_solver/Error.h"
#include <cstdlib>

namespace sat_solver {

    Literal::Literal(Int literal)
        : literal{literal} {
        if (literal == Literal::Terminator) {
            throw SatError{SatErrorCode::InvalidParameter, "Literal cannot be zero"};
        }
    }

    void swap(Literal &l1, Literal &l2) {
        std::swap(l1.literal, l2.literal);
    }
}
