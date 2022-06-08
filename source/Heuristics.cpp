/*
    SPDX-License-Identifier: MIT
    SPDX-FileCopyrightText: 2022 Protopopovs Jevgenijs
*/

#include "pjos/Heuristics.h"
#include <cassert>

namespace pjos {

    EVSIDSHeuristics::EVSIDSHeuristics(const Formula &formula, const Assignment &assignment, const ScoringParameters &scoring)
        : formula{formula}, assignment{assignment}, scoring{scoring} {
        
        this->FormulaUpdated();
    }

    void EVSIDSHeuristics::Reset() {
        std::size_t num_of_variables = this->formula.NumOfVariables();
        std::size_t num_of_scores = this->scores.size();

        for (std::size_t i = 0; i < num_of_scores; i++) {
            if (i < this->scores.size()) {
                this->scores[i] = 0;
            }
        }

        for (std::size_t i = num_of_scores; i < num_of_variables; i++) {
            this->scores.push_back(0);
        }

        this->ordered_variables.clear();
        for (Literal::UInt i = 1; i <= this->formula.NumOfVariables(); i++) {
            this->ordered_variables.push_back(i);
            this->ordered_variable_index.insert(i);
        }
        std::make_heap(this->ordered_variables.begin(), this->ordered_variables.end(), Comparator{*this});
        this->score_increment = this->scoring.initial_increment;
    }

    void EVSIDSHeuristics::FormulaUpdated() {
        std::size_t num_of_variables = this->formula.NumOfVariables();
        std::size_t num_of_scores = this->scores.size();
        
        if (num_of_scores < num_of_variables) {
            for (std::size_t i = num_of_scores; i < num_of_variables; i++) {
                this->scores.push_back(0);
                this->ordered_variables.push_back(i + 1);
                this->ordered_variable_index.insert(i + 1);
            }
        } else if (num_of_scores > num_of_variables) {
            this->scores.erase(this->scores.begin() + num_of_variables, this->scores.end());
            auto filter_fn = [num_of_variables](std::size_t variable) {
                return variable > num_of_variables;
            };

            auto iterator = std::remove_if(this->ordered_variables.begin(), this->ordered_variables.end(), filter_fn);
            for (auto it = iterator; it != this->ordered_variables.end(); std::advance(it, 1)) {
                this->ordered_variable_index.erase(*it);
            }
            this->ordered_variables.erase(iterator, this->ordered_variables.end());
        }

        std::make_heap(this->ordered_variables.begin(), this->ordered_variables.end(), Comparator{*this});
    }

    void EVSIDSHeuristics::NextIteration() {
        this->score_increment *= this->scoring.decay_rate;
    }

    void EVSIDSHeuristics::VariableActive(Literal::UInt variable) {
        auto &score = this->scores[variable - 1];
        score += this->score_increment;
        assert(!std::isinf(score));
        assert(score >= 0.0);
        if (score > this->scoring.rescore_threshold) {
            for (auto &score : this->scores) {
                score /= this->scoring.rescore_threshold;
                assert(!std::isinf(score));
                assert(score >= 0.0);
            }
            this->score_increment /= this->scoring.rescore_threshold;
        }

        std::make_heap(this->ordered_variables.begin(), this->ordered_variables.end(), Comparator{*this});
    }

    void EVSIDSHeuristics::VariableAssigned(Literal::UInt variable) {
        auto assn = this->assignment[variable];
        if (assn == VariableAssignment::Unassigned && this->ordered_variable_index.count(variable) == 0) {
            this->ordered_variables.push_back(variable);
            this->ordered_variable_index.insert(variable);
            std::push_heap(this->ordered_variables.begin(), this->ordered_variables.end(), Comparator{*this});
        }
    }
    
    Literal::UInt EVSIDSHeuristics::PopVariable() {
        while (!this->ordered_variables.empty()) {
            std::pop_heap(this->ordered_variables.begin(), this->ordered_variables.end(), Comparator{*this});
            auto variable = this->ordered_variables.back();
            this->ordered_variables.pop_back();
            this->ordered_variable_index.erase(variable);
            if (this->assignment[variable] == VariableAssignment::Unassigned) {
#ifdef PJOS_DEBUG_VALIDATIONS_ENABLE
                for (auto v : this->ordered_variables) {
                    assert(this->assignment[v] != VariableAssignment::Unassigned || this->scores[v - 1] <= this->scores[variable - 1]);
                }
#endif
                return variable;
            }
        }

        return Literal::Terminator;
    }

    bool EVSIDSHeuristics::Comparator::operator()(Literal::UInt variable1, Literal::UInt variable2) const {
        double score1 = this->evsids.scores[variable1 - 1];
        double score2 = this->evsids.scores[variable2 - 1];

        return score1 < score2 || (!(score1 > score2) && variable1 < variable2);
    }
}