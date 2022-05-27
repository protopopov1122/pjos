/*
    SPDX-License-Identifier: MIT
    SPDX-FileCopyrightText: 2022 Protopopovs Jevgenijs
*/

#include "pjos/Formula.h"
#include <algorithm>

namespace pjos {
    
    Formula::IteratorType Formula::begin() const {
        return IteratorType{this->clauses.begin()};
    }

    Formula::IteratorType Formula::end() const {
        return IteratorType{this->clauses.end()};
    }

    const ClauseView &Formula::AppendClause(Clause clause) {
        const auto &view = this->clauses.emplace_back(std::move(clause));
        this->num_of_variables = std::max(this->num_of_variables, view.NumOfVariables());
        return view;
    }

    bool Formula::RemoveClause(std::size_t index) {
        if (index < this->clauses.size()) {
            this->clauses.erase(this->clauses.begin() + index);
            return true;
        } else {
            return false;
        }
    }

    FormulaBuilder::FormulaBuilder(Formula &formula)
        : formula{formula}, clause_builder{}, new_clause{true} {}

    FormulaBuilder::~FormulaBuilder() {
        this->Finish();
    }

    void FormulaBuilder::AppendLiteral(Literal literal) {
        this->new_clause = false;
        this->clause_builder.Add(literal);
    }

    void FormulaBuilder::EndClause() {
        this->formula.AppendClause(this->clause_builder.Make());
        this->new_clause = true;
    }

    void FormulaBuilder::Finish() {
        if (!this->new_clause) {
            this->formula.AppendClause(this->clause_builder.Make());
        }
    }
}
