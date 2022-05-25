#include "pjos/Clause.h"
#include "pjos/Error.h"
#include <algorithm>

namespace pjos {

    static bool var_comparator(const Literal &l1, const Literal &l2) {
        return l1.Variable() < l2.Variable();
    }

    ClauseView::ClauseView(const Literal *clause, std::size_t clause_length, Literal::UInt num_of_variables)
        : clause{clause}, clause_length{clause_length}, num_of_variables{num_of_variables} {}

    bool ClauseView::HasVariable(Literal::UInt var) const {
        return std::binary_search(this->begin(), this->end(), var, var_comparator);
    }

    ClauseView::IteratorType ClauseView::FindLiteral(Literal literal) const {
        auto it = std::lower_bound(this->begin(), this->end(), literal, std::less<Literal>{});
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

    Clause::Clause(std::unique_ptr<Literal[]> clause, std::size_t clause_length, Literal::UInt num_of_variables)
        : ClauseView::ClauseView{clause.get(), clause_length, num_of_variables}, clause{std::move(clause)} {}
    
    Clause::Clause(const ClauseView &other)
        : ClauseView{nullptr, other.Length(), other.NumOfVariables()}, clause{nullptr} {
        
        this->clause = std::make_unique<Literal[]>(other.Length());
        this->ClauseView::clause = this->clause.get();
        std::copy_n(other.begin(), this->clause_length, this->clause.get());
    }

    Clause &Clause::operator=(const Clause &other) {
        this->clause = std::make_unique<Literal[]>(other.clause_length);
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

    Clause ClauseBuilder::Make() {
        auto clause_literals = std::make_unique<Literal[]>(this->literals.size());
        std::copy(this->literals.begin(), this->literals.end(), clause_literals.get());
        auto clause = Clause{std::move(clause_literals), this->literals.size(), this->num_of_variables};
        this->Reset();
        return clause;
    }
}
