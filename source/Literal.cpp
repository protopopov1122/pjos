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

    Literal::Int Literal::Get() const {
        return this->literal;
    }

    Literal::Int Literal::Variable() const {
        return std::abs(this->literal);
    }

    LiteralPolarity Literal::Polarity() const {
        return this->literal < 0 ? LiteralPolarity::Negative : LiteralPolarity::Positive;
    }

    Literal::operator Int() const {
        return this->Get();
    }

    bool Literal::operator==(const Literal &l) const {
        return this->literal == l.literal;
    }

    bool Literal::operator!=(const Literal &l) const {
        return this->literal != l.literal;
    }

    void swap(Literal &l1, Literal &l2) {
        std::swap(l1.literal, l2.literal);
    }
}

bool std::less<sat_solver::Literal>::operator()(const sat_solver::Literal &l1, const sat_solver::Literal &l2) const noexcept {
    return l1.Get() < l2.Get();
}

std::size_t std::hash<sat_solver::Literal>::operator()(const sat_solver::Literal &l) const noexcept {
    return static_cast<std::size_t>(l.Get());
}
