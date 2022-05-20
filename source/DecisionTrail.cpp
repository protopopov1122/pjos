#include "sat_solver/DecisionTrail.h"
#include <algorithm>

namespace sat_solver {

    static constexpr std::size_t EmptyIndex = static_cast<std::size_t>(-1);

    DecisionTrail::Entry::Entry(Literal::Int variable, VariableAssignment assn, Reason reason, std::size_t level)
        : variable{variable}, assignment{assn}, reason{reason}, level{level} {}

    DecisionTrail::DecisionTrail(std::size_t num_of_variables)
        : var_index(num_of_variables, EmptyIndex) {}

    void DecisionTrail::Decision(Literal::Int variable, VariableAssignment assn) {
        this->var_index[variable - 1] = this->trail.size();
        this->trail.emplace_back(variable, assn, ReasonDecision, ++this->level);
    }

    void DecisionTrail::Propagation(Literal::Int variable, VariableAssignment assn) {
        this->var_index[variable - 1] = this->trail.size();
        this->trail.emplace_back(variable, assn, ReasonPropagation, this->level);
    }

    void DecisionTrail::Propagation(Literal::Int variable, VariableAssignment assn, std::size_t reason) {
        this->var_index[variable - 1] = this->trail.size();
        this->trail.emplace_back(variable, assn, static_cast<Reason>(reason), this->level);
    }

    std::optional<DecisionTrail::Entry> DecisionTrail::Undo() {
        while (!this->trail.empty() && static_cast<std::size_t>(this->trail.back().variable) > this->var_index.size()) {
            this->trail.pop_back();
        }
        if (this->trail.empty()) {
            return std::optional<DecisionTrail::Entry>{};
        }

        auto entry = this->trail.back();
        this->trail.pop_back();
        this->var_index[entry.variable - 1] = EmptyIndex;
        if (!this->trail.empty()) {
            this->level = this->trail.back().level;
        } else {
            this->level = 0;
        }
        return entry;
    }

    void DecisionTrail::SetNumOfVariables(std::size_t num_of_variables) {
        if (this->var_index.size() > num_of_variables) {
            this->var_index.erase(this->var_index.begin() + num_of_variables, this->var_index.end());
        } else if (this->var_index.size() < num_of_variables) {
            this->var_index.insert(this->var_index.end(), num_of_variables - this->var_index.size(), EmptyIndex);
        }
    }

    const DecisionTrail::Entry *DecisionTrail::Find(Literal::Int variable) const {
        if (variable - 1 >= static_cast<std::int64_t>(this->var_index.size())) {
            return nullptr;
        }
        auto index = this->var_index[variable - 1];
        if (index != EmptyIndex) {
            return std::addressof(this->trail[index]);
        } else {
            return nullptr;
        }
    }
}
