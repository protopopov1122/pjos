#ifndef SAT_SOLVER_HEURISTICS_H_
#define SAT_SOLVER_HEURISTICS_H_

#include "sat_solver/Formula.h"
#include "sat_solver/Assignment.h"
#include <algorithm>

namespace sat_solver {

    template <typename T, std::int64_t ScoreIncrement, std::int64_t RescoreThreshold, std::int64_t RescoreFactor>
    class VSIDSHeuristics {
     public:
        VSIDSHeuristics(const Formula &formula, const Assignment &assignment, T variable_stats)
            : formula{formula}, assignment{assignment}, variable_stats{variable_stats} {
            
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
            for (Literal::Int i = 1; i <= this->formula.NumOfVariables(); i++) {
                this->ordered_variables.push_back(i);
            }
            std::make_heap(this->ordered_variables.begin(), this->ordered_variables.end(), Comparator{*this});
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
        }

        void VariableActive(Literal::Int variable) {
            auto &score = this->scores[variable - 1];
            score += ScoreIncrement;
            if (score > RescoreThreshold) {
                for (auto &score : this->scores) {
                    score >>= RescoreFactor;
                }
            }

            std::make_heap(this->ordered_variables.begin(), this->ordered_variables.end(), Comparator{*this});
        }

        void VariableAssigned(Literal::Int) {
            std::make_heap(this->ordered_variables.begin(), this->ordered_variables.end(), Comparator{*this});
        }
     
        Literal::Int TopVariable() {
            if (this->ordered_variables.empty()) {
                return Literal::Terminator;
            } else {
                return this->ordered_variables.front();
            }
        }

     private:
        struct Comparator {
            bool operator()(Literal::Int variable1, Literal::Int variable2) const {
                auto score1 = this->vsids.assignment[variable1] == VariableAssignment::Unassigned
                    ? this->vsids.scores[variable1 - 1]
                    : -1;
                auto score2 = this->vsids.assignment[variable2] == VariableAssignment::Unassigned
                    ? this->vsids.scores[variable2 - 1]
                    : -1;

                return score1 < score2 || (score1 == score2 && variable1 >= variable2);
            }

            VSIDSHeuristics &vsids;
        };

        const Formula &formula;
        const Assignment &assignment;
        T variable_stats;
        std::vector<std::int64_t> scores;
        std::vector<Literal::Int> ordered_variables;
    };
}

#endif
