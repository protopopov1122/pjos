/*
    SPDX-License-Identifier: MIT
    SPDX-FileCopyrightText: 2022 Protopopovs Jevgenijs
*/

#ifndef PJOS_HEURISTICS_H_
#define PJOS_HEURISTICS_H_

#include "pjos/Formula.h"
#include "pjos/Assignment.h"
#include <algorithm>

namespace pjos {

    template <typename T>
    class EVSIDSHeuristics {
     public:
        struct ScoringParameters {
            double rescore_threshold;
            double rescore_factor;
            double initial_increment;
            double decay_rate;
        };

        EVSIDSHeuristics(const Formula &formula, const Assignment &assignment, const ScoringParameters &scoring, T variable_stats)
            : formula{formula}, assignment{assignment}, scoring{scoring}, variable_stats{variable_stats} {
            
            this->FormulaUpdated();
        }

        void Reset() {
            std::size_t num_of_variables = this->formula.NumOfVariables();
            std::size_t num_of_scores = this->scores.size();

            for (std::size_t i = 0; i < num_of_scores; i++) {
                if (i < this->scores.size()) {
                    this->scores[i] = this->variable_stats(i + 1);
                }
            }

            for (std::size_t i = num_of_scores; i < num_of_variables; i++) {
                this->scores.push_back(this->variable_stats(i + 1));
            }

            this->ordered_variables.clear();
            for (Literal::UInt i = 1; i <= this->formula.NumOfVariables(); i++) {
                this->ordered_variables.push_back(i);
            }
            std::make_heap(this->ordered_variables.begin(), this->ordered_variables.end(), Comparator{*this});
            this->score_increment = this->scoring.initial_increment;
            this->update_heap = false;
        }

        void FormulaUpdated() {
            std::size_t num_of_variables = this->formula.NumOfVariables();
            std::size_t num_of_scores = this->scores.size();
            
            if (num_of_scores < num_of_variables) {
                for (std::size_t i = num_of_scores; i < num_of_variables; i++) {
                    this->scores.push_back(this->variable_stats(i + 1));
                    this->ordered_variables.push_back(i + 1);
                }
            } else if (num_of_scores > num_of_variables) {
                this->scores.erase(this->scores.begin() + num_of_variables, this->scores.end());
                auto iterator = std::remove_if(this->ordered_variables.begin(), this->ordered_variables.end(), [num_of_variables](std::size_t variable) {
                    return variable > num_of_variables;
                });
                this->ordered_variables.erase(iterator, this->ordered_variables.end());
            }

            std::make_heap(this->ordered_variables.begin(), this->ordered_variables.end(), Comparator{*this});
            this->update_heap = false;
        }

        void NextIteration() {
            this->score_increment *= this->scoring.decay_rate;
        }

        void VariableActive(Literal::UInt variable) {
            auto &score = this->scores[variable - 1];
            score += this->score_increment;
            if (score > this->scoring.rescore_threshold) {
                for (auto &score : this->scores) {
                    score *= this->scoring.rescore_threshold;
                }
                this->score_increment = this->scoring.initial_increment;
            }

            this->update_heap = true;
        }

        void VariableAssigned(Literal::UInt) {
            this->update_heap = true;
        }
     
        Literal::UInt TopVariable() {
            if (this->update_heap) {
                std::make_heap(this->ordered_variables.begin(), this->ordered_variables.end(), Comparator{*this});
                this->update_heap = false;
            }
            if (this->ordered_variables.empty()) {
                return Literal::Terminator;
            } else {
                return this->ordered_variables.front();
            }
        }

     private:
        struct Comparator {
            bool operator()(Literal::UInt variable1, Literal::UInt variable2) const {
                auto score1 = this->evsids.assignment[variable1] == VariableAssignment::Unassigned
                    ? this->evsids.scores[variable1 - 1]
                    : -1.0;
                auto score2 = this->evsids.assignment[variable2] == VariableAssignment::Unassigned
                    ? this->evsids.scores[variable2 - 1]
                    : -1.0;

                return score1 < score2 || (score1 == score2 && variable1 >= variable2);
            }

            EVSIDSHeuristics<T> &evsids;
        };

        const Formula &formula;
        const Assignment &assignment;
        ScoringParameters scoring;
        T variable_stats;
        std::vector<double> scores;
        double score_increment{1.0};
        std::vector<Literal::UInt> ordered_variables;
        bool update_heap{false};
    };
}

#endif
