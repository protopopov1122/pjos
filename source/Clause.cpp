/*
    SPDX-License-Identifier: MIT
    SPDX-FileCopyrightText: 2022 Protopopovs Jevgenijs
*/

#include "pjos/Clause.h"
#include "pjos/Error.h"
#include <algorithm>

namespace pjos {

    ClauseView::ClauseView(const Literal *clause, std::size_t clause_length, Literal::UInt num_of_variables)
        : clause{clause}, clause_length{clause_length}, num_of_variables{num_of_variables} {}

    bool ClauseView::HasVariable(Literal::UInt var) const {
        return std::find_if(this->begin(), this->end(), [var](Literal l) {
            return l.Variable() == var;
        }) != this->end();
    }

    ClauseView::IteratorType ClauseView::FindLiteral(Literal literal) const {
        auto it = std::find(this->begin(), this->end(), literal);
        if (it != this->end() && literal == *it) {
            return it;
        } else {
            return this->end();
        }
    }

    Literal ClauseView::At(std::size_t index) const {
        if (index < this->clause_length) {
            return this->clause[index];
        } else {
            throw SatError{"Requested literal index is out of bounds"};
        }
    }

    void swap(ClauseView &c1, ClauseView &c2) {
        std::swap(c1.clause, c2.clause);
        std::swap(c1.clause_length, c2.clause_length);
        std::swap(c1.num_of_variables, c2.num_of_variables);
    }
    
    void Clause::LiteralsDeleter::operator()(Literal literals[]) const {
        if (this->owner) {
            delete[] literals;
        }
    }

    Clause::Clause(std::unique_ptr<Literal[], LiteralsDeleter> clause, std::size_t clause_length, Literal::UInt num_of_variables)
        : ClauseView::ClauseView{clause.get(), clause_length, num_of_variables}, clause{std::move(clause)} {}
    
    Clause::Clause(const ClauseView &other)
        : ClauseView{nullptr, other.Length(), other.NumOfVariables()}, clause{nullptr} {
        
        this->clause = std::unique_ptr<Literal[], LiteralsDeleter>(new Literal[other.Length()], LiteralsDeleter{true});
        this->ClauseView::clause = this->clause.get();
        std::copy_n(other.begin(), this->clause_length, this->clause.get());
    }

    Clause &Clause::operator=(const Clause &other) {
        this->clause = std::unique_ptr<Literal[], LiteralsDeleter>(new Literal[other.clause_length], LiteralsDeleter{true});
        this->ClauseView::clause = this->clause.get();
        this->clause_length = other.clause_length;
        this->num_of_variables = other.num_of_variables;
        std::copy_n(other.begin(), this->clause_length, this->clause.get());
        return *this;
    }

    void swap(Clause &c1, Clause &c2) {
        std::swap(c1.ClauseView::clause, c2.ClauseView::clause);
        std::swap(c1.clause, c2.clause);
        std::swap(c1.clause_length, c2.clause_length);
        std::swap(c1.num_of_variables, c2.num_of_variables);
    }

    ClauseBuilder &ClauseBuilder::Add(Literal literal) {
        this->literals.insert(literal);
        this->num_of_variables = std::max(literal.Variable(), this->num_of_variables);
        return *this;
    }

    ClauseBuilder &ClauseBuilder::Reset() {
        this->literals.clear();
        this->num_of_variables = 0;
        return *this;
    }

    struct DefaultLiteralAllocator {
        auto operator()(std::size_t length) const {
            return std::unique_ptr<Literal[], Clause::LiteralsDeleter>(new Literal[length], Clause::LiteralsDeleter{true});
        }
    };

    Clause ClauseBuilder::Make() {
        static DefaultLiteralAllocator allocator{};
        return this->Make(allocator);
    }
}
