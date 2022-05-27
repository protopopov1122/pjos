/*
    SPDX-License-Identifier: MIT
    SPDX-FileCopyrightText: 2022 Protopopovs Jevgenijs
*/

#include "pjos/DecisionTrail.h"
#include <algorithm>

namespace pjos {

    static constexpr std::size_t EmptyIndex = static_cast<std::size_t>(-1);

    DecisionTrail::Entry::Entry(Literal::UInt variable, VariableAssignment assn, Reason reason, std::size_t level)
        : variable{variable}, assignment{assn}, reason{reason}, level{level} {}

    DecisionTrail::DecisionTrail(std::size_t num_of_variables)
        : var_index(num_of_variables, EmptyIndex) {}

    void DecisionTrail::Decision(Literal::UInt variable, VariableAssignment assn) {
        this->var_index[variable - 1] = this->trail.size();
        this->trail.emplace_back(variable, assn, ReasonDecision, ++this->level);
    }

    void DecisionTrail::Propagation(Literal::UInt variable, VariableAssignment assn) {
        this->var_index[variable - 1] = this->trail.size();
        this->trail.emplace_back(variable, assn, ReasonPropagation, this->level);
    }

    void DecisionTrail::Propagation(Literal::UInt variable, VariableAssignment assn, std::size_t reason) {
        this->var_index[variable - 1] = this->trail.size();
        this->trail.emplace_back(variable, assn, static_cast<Reason>(reason), this->level);
    }

    void DecisionTrail::Assumption(Literal::UInt variable, VariableAssignment assn) {
        this->var_index[variable - 1] = this->trail.size();
        this->trail.emplace_back(variable, assn, ReasonAssumption, ++this->level);
    }

    const DecisionTrail::Entry *DecisionTrail::Top() {
        while (!this->trail.empty() && this->trail.back().variable > this->var_index.size()) {
            this->trail.pop_back();
        }
        if (!this->trail.empty()) {
            return std::addressof(this->trail.back());
        } else {
            return nullptr;
        }
    }

    void DecisionTrail::Pop() {
        while (!this->trail.empty() && this->trail.back().variable > this->var_index.size()) {
            this->trail.pop_back();
        }
        if (!this->trail.empty()) {
            auto &entry = this->trail.back();
            this->var_index[entry.variable - 1] = EmptyIndex;
            this->trail.pop_back();
            if (!this->trail.empty()) {
                this->level = this->trail.back().level;
            } else {
                this->level = 0;
            }
        }
    }

    void DecisionTrail::Reset() {
        this->trail.clear();
        this->level = 0;
        std::fill(this->var_index.begin(), this->var_index.end(), EmptyIndex);
    }

    void DecisionTrail::SetNumOfVariables(std::size_t num_of_variables) {
        if (this->var_index.size() > num_of_variables) {
            this->var_index.erase(this->var_index.begin() + num_of_variables, this->var_index.end());
        } else if (this->var_index.size() < num_of_variables) {
            this->var_index.insert(this->var_index.end(), num_of_variables - this->var_index.size(), EmptyIndex);
        }
    }

    const DecisionTrail::Entry *DecisionTrail::Find(Literal::UInt variable) const {
        if (variable - 1 >= this->var_index.size()) {
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
