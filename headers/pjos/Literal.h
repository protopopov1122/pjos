/*
    SPDX-License-Identifier: MIT
    SPDX-FileCopyrightText: 2022 Protopopovs Jevgenijs
*/

#ifndef PJOS_LITERAL_H_
#define PJOS_LITERAL_H_

#include "pjos/Core.h"
#include <cinttypes>
#include <cstdlib>
#include <functional>
#include <utility>

namespace pjos {

    inline constexpr VariableAssignment FlipVariableAssignment(VariableAssignment assn) {
        switch (assn) {            
            case VariableAssignment::False:
                return VariableAssignment::True;

            case VariableAssignment::True:
                return VariableAssignment::False;

            case VariableAssignment::Unassigned:
                // Intentionally left blank
                break;
        }
        return VariableAssignment::Unassigned;
    }

    class Literal {
     public:
        using Int = std::int_fast64_t;
        using UInt = std::uint_fast64_t;

        Literal() = default;
        Literal(const Literal &) = default;
        Literal(Literal &&) = default;


        Literal(Int literal)
            : literal{literal} {
#ifdef PJOS_HOTPATH_PARAM_CHECKS
            if (literal == Literal::Terminator) {
                throw SatError{"Literal cannot be zero"};
            }
#endif
        }

        Literal(UInt literal, VariableAssignment assn) {
#ifdef PJOS_HOTPATH_PARAM_CHECKS
            if (literal == Literal::Terminator) {
                throw SatError{"Literal cannot be zero"};
            }
#endif

            if (assn == VariableAssignment::False) {
                this->literal = -1 * static_cast<Int>(literal);
            } else {   
                this->literal = static_cast<Int>(literal);
            }
        }

        ~Literal() = default;

        Literal &operator=(const Literal &) = default;
        Literal &operator=(Literal &&) = default;

        inline Int Get() const {
            return this->literal;
        }

        inline UInt Variable() const {
            return std::abs(this->literal);
        }

        inline std::pair<UInt, VariableAssignment> Assignment() const {
            return std::make_pair(this->Variable(), this->literal < 0
                ? VariableAssignment::False
                : VariableAssignment::True);
        }

        inline bool Eval(VariableAssignment assn) const {
            return (this->literal > 0 && assn == VariableAssignment::True) ||
                   (this->literal < 0 && assn == VariableAssignment::False);
        }

        inline Literal Negate() const {
            return Literal{-this->literal};
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

        friend void swap(Literal &l1, Literal &l2) {
            std::swap(l1.literal, l2.literal);
        }

        static constexpr Literal::Int Terminator{0};

     private:
        Int literal{Literal::Terminator};
    };
}

template <>
struct std::hash<pjos::Literal> {
    std::size_t operator()(const pjos::Literal &l) const noexcept {
        return static_cast<std::size_t>(l.Get());
    }
};

template <>
struct std::less<pjos::Literal> {
    bool operator()(const pjos::Literal &l1, const pjos::Literal &l2) const noexcept {
        return l1.Variable() < l2.Variable() || (l1.Variable() == l2.Variable() && l1.Get() < l2.Get());
    }
};

#endif
