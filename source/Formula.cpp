#include "sat_solver/Formula.h"
#include <algorithm>

namespace sat_solver {

    Formula::Formula(ClauseContainer &clause_container)
        : clause_container{std::addressof(clause_container)} {}
    
    Formula::IteratorType Formula::begin() const {
        return IteratorType{this->clauses.begin()};
    }

    Formula::IteratorType Formula::end() const {
        return IteratorType{this->clauses.end()};
    }

    const ClauseView &Formula::AppendClause(Clause clause) {
        const auto &view = *this->clauses.emplace_back(this->clause_container->Attach(std::move(clause)));
        this->num_of_variables = std::max(this->num_of_variables, view.NumOfVariables());
        return view;
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
