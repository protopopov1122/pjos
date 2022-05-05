#ifndef SAT_SOLVER_LITERAL_H_
#define SAT_SOLVER_LITERAL_H_

#include "sat_solver/Base.h"
#include <cinttypes>
#include <cstdlib>
#include <functional>

namespace sat_solver {

    enum class LiteralPolarity {
        Positive,
        Negative
    };

    class Literal {
     public:
        using Int = std::int_fast64_t;

        Literal() = default;
        Literal(Int);
        Literal(const Literal &) = default;
        Literal(Literal &&) = default;

        ~Literal() = default;

        Literal &operator=(const Literal &) = default;
        Literal &operator=(Literal &&) = default;

        inline Int Get() const {
            return this->literal;
        }

        inline Int Variable() const {
            return std::abs(this->literal);
        }

        inline LiteralPolarity Polarity() const {
            return this->literal < 0
                ? LiteralPolarity::Negative
                : LiteralPolarity::Positive;
        }

        inline explicit operator Int() const {
            return this->literal;
        }

        inline bool operator==(const Literal &other) const {
            return this->literal == other.literal;
        }

        inline bool operator!=(const Literal &other) const {
            return this->literal != other.literal;
        }

        friend void swap(Literal &, Literal &);

        static constexpr Literal::Int Terminator{0};

     private:
        Int literal{Literal::Terminator};
    };
}

template <>
struct std::hash<sat_solver::Literal> {
    std::size_t operator()(const sat_solver::Literal &l) const noexcept {
        return static_cast<std::size_t>(l.Get());
    }
};

template <>
struct std::less<sat_solver::Literal> {
    bool operator()(const sat_solver::Literal &l1, const sat_solver::Literal &l2) const noexcept {
        return l1.Variable() < l2.Variable() || (l1.Variable() == l2.Variable() && l1.Get() < l2.Get());
    }
};

#endif
