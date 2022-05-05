#ifndef SAT_SOLVER_LITERAL_H_
#define SAT_SOLVER_LITERAL_H_

#include "sat_solver/Base.h"
#include <cinttypes>
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

        Int Get() const;
        Int Variable() const;
        LiteralPolarity Polarity() const;

        explicit operator Int() const;

        bool operator==(const Literal &) const;
        bool operator!=(const Literal &) const;

        friend void swap(Literal &, Literal &);

        static constexpr Literal::Int Terminator{0};

     private:
        Int literal{Literal::Terminator};
    };
}

template <>
struct std::hash<sat_solver::Literal> {
    std::size_t operator()(const sat_solver::Literal &) const noexcept;
};

template <>
struct std::less<sat_solver::Literal> {
    bool operator()(const sat_solver::Literal &, const sat_solver::Literal &) const noexcept;
};

#endif
