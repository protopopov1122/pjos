#include "sat_solver/Formula.h"
#include <algorithm>

namespace sat_solver {

    Formula::Formula(ClauseContainer &clause_container)
        : clause_container{std::addressof(clause_container)} {}

    std::size_t Formula::NumOfClauses() const {
        return this->clauses.size();
    }

    Literal::Int Formula::NumOfVariables() const {
        auto it = std::max_element(this->begin(), this->end(), [](const auto &c1, const auto &c2) {
            return c1.NumOfVariables() < c2.NumOfVariables();
        });
        if (it != this->end()) {
            return it->NumOfVariables();
        } else {
            return Literal::Terminator;
        }
    }

    const ClauseView &Formula::At(std::size_t index) const {
        return *this->clauses.at(index);
    }

    Formula::IteratorType Formula::begin() const {
        return IteratorType{this->clauses.begin()};
    }

    Formula::IteratorType Formula::end() const {
        return IteratorType{this->clauses.end()};
    }

    const ClauseView &Formula::AppendClause(Clause clause) {
        return *this->clauses.emplace_back(this->clause_container->Attach(std::move(clause)));
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
