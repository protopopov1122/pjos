/*
    SPDX-License-Identifier: MIT
    SPDX-FileCopyrightText: 2022 Protopopovs Jevgenijs
*/

#ifndef PJOS_FORMULA_H_
#define PJOS_FORMULA_H_

#include "pjos/Clause.h"
#include <iterator>
#include <vector>

// Formula contains an array of clauses. It can be traversed via
// iterators or random access. Formula owns clauses that it contains.
// Content of clauses in the formula is immutable, however formula itself
// can be modified by adding/removing clauses. Additionally, formula
// keeps track of current number of variables used by clauses, as it
// is useful information to be accessed in constant time.

namespace pjos {

    namespace internal {
        template <typename I>
        class FormulaIterator {
            using difference_type = typename std::iterator_traits<FormulaIterator<I>>::difference_type;

         public:
            FormulaIterator(I iter)
                : iter{std::move(iter)} {}
            FormulaIterator(const FormulaIterator<I> &) = default;
            FormulaIterator(FormulaIterator<I> &&) = default;

            ~FormulaIterator() = default;

            FormulaIterator<I> &operator=(const FormulaIterator<I> &) = default;
            FormulaIterator<I> &operator=(FormulaIterator<I> &&) = default;

            FormulaIterator<I> &operator++() {
                std::advance(this->iter, 1);
                return *this;
            }

            FormulaIterator<I> &operator--() {
                std::advance(this->iter, -1);
                return *this;
            }

            FormulaIterator<I> operator++(int) {
                auto clone(*this);
                this->operator++();
                return clone;
            }

            FormulaIterator<I> operator--(int) {
                auto clone(*this);
                this->operator--();
                return clone;
            }

            FormulaIterator<I> &operator+=(difference_type distance) {
                std::advance(this->iter, distance);
                return *this;
            }

            FormulaIterator<I> &operator-=(difference_type distance) {
                std::advance(this->iter, -distance);
                return *this;
            }

            FormulaIterator<I> operator+(difference_type distance) const {
                return FormulaIterator{std::next(this->iter, distance)};
            }

            FormulaIterator<I> operator-(difference_type distance) const {
                return FormulaIterator{std::prev(this->iter, distance)};
            }

            difference_type operator-(const FormulaIterator<I> &other) const {
                return std::distance(this->iter, other.iter);
            }

            bool operator==(const FormulaIterator<I> &other) const {
                return this->iter == other.iter;
            }

            bool operator!=(const FormulaIterator<I> &other) const {
                return this->iter != other.iter;
            }

            const ClauseView &operator*() const {
                return *iter;
            }

            const ClauseView *operator->() const {
                return std::addressof(*iter);
            }

            friend void swap(FormulaIterator<I> &it1, FormulaIterator<I> &it2) {
                std::swap(it1.iter, it2.iter);
            }

         private:
            I iter;
        };
    }

    class Formula {
     public:
        using IteratorType = internal::FormulaIterator<std::vector<Clause>::const_iterator>;

        Formula() = default;
        Formula(const Formula &) = default;
        Formula(Formula &&) = default;

        ~Formula() = default;

        Formula &operator=(const Formula &) = default;
        Formula &operator=(Formula &&) = default;

        inline std::size_t NumOfClauses() const {
            return this->clauses.size();
        }

        inline bool Empty() const {
           return this->clauses.empty();
        }

        inline Literal::UInt NumOfVariables() const {
            return this->num_of_variables;
        }

        const ClauseView &At(std::size_t index) const;

        inline const ClauseView &operator[](std::size_t index) const {
            return this->clauses[index];
        }

        IteratorType begin() const;
        IteratorType end() const;

        const ClauseView &AppendClause(Clause);
        bool RemoveClause(std::size_t);

     private:
        std::vector<Clause> clauses{};
        Literal::UInt num_of_variables{0};
    };

    class FormulaBuilder { // Helper to build formulas for literal stream
     public:
        FormulaBuilder(Formula &);
        FormulaBuilder(const FormulaBuilder &) = delete;
        FormulaBuilder(FormulaBuilder &&) = delete;

        ~FormulaBuilder();

        FormulaBuilder &operator=(const FormulaBuilder &) = delete;
        FormulaBuilder &operator=(FormulaBuilder &&) = delete;

        void AppendLiteral(Literal);
        void EndClause();
        void Finish();

     private:
        Formula &formula;
        ClauseBuilder clause_builder;
        bool new_clause;
    }; 
}

template <typename I>
struct std::iterator_traits<pjos::internal::FormulaIterator<I>> {
    using difference_type = std::ptrdiff_t;
    using value_type = const pjos::ClauseView;
    using pointer = const pjos::ClauseView *;
    using reference = const pjos::ClauseView &;
    using iterator_category = std::bidirectional_iterator_tag;
};

#endif
