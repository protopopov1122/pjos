/*
    SPDX-License-Identifier: MIT
    SPDX-FileCopyrightText: 2022 Protopopovs Jevgenijs
*/

#ifndef PJOS_CLAUSE_H_
#define PJOS_CLAUSE_H_

// Data structures that represent formula clauses.
// Each clause consists of a set of unique literals (duplicate literals
// bring no value). Clauses are immutable, can be traversed using
// iterators, provide random access to elements and have literal/variable
// search capability.

#include "pjos/Literal.h"
#include <memory>
#include <unordered_set>

namespace pjos {

    class ClauseView { // Providing operations on clause in non-owned memory block.
                       // Cheap to copy, however shall never outline clause owner.
     public:
        using IteratorType = const Literal *;

        ClauseView() = delete;
        ClauseView(const ClauseView &) = default;
        ClauseView(ClauseView &&) = default;

        ~ClauseView() = default;

        ClauseView &operator=(const ClauseView &) = default;
        ClauseView &operator=(ClauseView &&) = default;

        bool HasVariable(Literal::UInt) const;
        IteratorType FindLiteral(Literal) const;

        inline bool HasLiteral(Literal literal) const {
            return this->FindLiteral(literal) != this->end();
        }

        inline std::size_t Length() const {
           return this->clause_length;
        }

        inline bool Empty() const {
           return this->clause_length == 0;
        }

        inline Literal::UInt NumOfVariables() const {
           return this->num_of_variables;
        }

        Literal At(std::size_t) const;

        inline Literal operator[](std::size_t index) const {
           return this->clause[index];
        }

        inline IteratorType begin() const {
           return this->clause;
        }

        inline IteratorType end() const {
           return this->clause + this->clause_length;
        }
    
        friend void swap(ClauseView &, ClauseView &);

     protected:
        ClauseView(const Literal *, std::size_t, Literal::UInt);

        const Literal *clause;
        std::size_t clause_length;
        Literal::UInt num_of_variables;
    };

    class Clause : public ClauseView { // Clause owning it's own memory
     public:
        struct LiteralsDeleter {
            bool owner;

            void operator()(Literal[]) const;
        };

        Clause(const ClauseView &);
        Clause(Clause &&) = default;

        ~Clause() = default;

        Clause &operator=(const Clause &);
        Clause &operator=(Clause &&) = default;

        inline const ClauseView &View() const {
           return *this;
        }

        friend void swap(Clause &, Clause &);
        friend class ClauseBuilder;

     private:
        Clause(std::unique_ptr<Literal[], LiteralsDeleter>, std::size_t, Literal::UInt);

        std::unique_ptr<Literal[], LiteralsDeleter> clause;
    };

    class ClauseBuilder { // Helper class for building clauses.
                          // For allocation performance reasons it might be beneficial to keep
                          // the builder in static memory/class field and reuse it,
                          // avoidit allocating it on stack
     public:
        ClauseBuilder() = default;
        ClauseBuilder(const ClauseBuilder &) = delete;
        ClauseBuilder(ClauseBuilder &&) = delete;

        ~ClauseBuilder() = default;

        ClauseBuilder &operator=(const ClauseBuilder &) = delete;
        ClauseBuilder &operator=(ClauseBuilder &&) = delete;

        ClauseBuilder &Add(Literal);
        ClauseBuilder &Reset();
        Clause Make();

        template <typename T>
        Clause Make(T &allocator) {
            auto clause_literals = allocator(this->literals.size());
            std::copy(this->literals.begin(), this->literals.end(), clause_literals.get());
            auto clause = Clause{std::move(clause_literals), this->literals.size(), this->num_of_variables};
            this->Reset();
            return clause;
        }

     private:
        std::unordered_set<Literal> literals;
        Literal::UInt num_of_variables{0};
    };
}

#endif
