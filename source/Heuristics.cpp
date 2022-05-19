#include "sat_solver/Heuristics.h"
#include <algorithm>

namespace sat_solver {

    static constexpr std::int64_t ScoreIncrement = 1;
    static constexpr std::int64_t RescoreThreshold = 1024;
    static constexpr std::int64_t RescoreFactor = 1;

    bool VSIDSHeuristics::Comparator::operator()(Literal::Int variable1, Literal::Int variable2) const {
        auto score1 = this->vsids.assignment[variable1] == VariableAssignment::Unassigned
            ? this->vsids.scores[variable1 - 1]
            : -1;
        auto score2 = this->vsids.assignment[variable2] == VariableAssignment::Unassigned
            ? this->vsids.scores[variable2 - 1]
            : -1;

        return score1 < score2 || (score1 == score2 && variable1 >= variable2);
    }

    VSIDSHeuristics::VSIDSHeuristics(const Formula &formula, const Assignment &assignment)
        : formula{formula}, assignment{assignment} {
        
        this->OnUpdate();
    }

    void VSIDSHeuristics::Reset() {
        std::size_t num_of_variables = this->formula.NumOfVariables();
        std::size_t num_of_scores = this->scores.size();
        if (num_of_variables > num_of_scores) {
            this->scores.insert(this->scores.end(), num_of_variables - num_of_scores, 0);
        }
        std::fill_n(this->scores.begin(), num_of_scores, 0);

        this->ordered_variables.clear();
        for (Literal::Int i = 1; i <= this->formula.NumOfVariables(); i++) {
            this->ordered_variables.push_back(i);
        }
        std::make_heap(this->ordered_variables.begin(), this->ordered_variables.end(), Comparator{*this});
    }

    void VSIDSHeuristics::OnUpdate() {
        std::size_t num_of_variables = this->formula.NumOfVariables();
        std::size_t num_of_scores = this->scores.size();
        if (num_of_variables > num_of_scores) {
            this->scores.insert(this->scores.end(), num_of_variables - num_of_scores, 0);
        }

        for (Literal::Int i = num_of_scores + 1; i <= this->formula.NumOfVariables(); i++) {
            this->ordered_variables.push_back(i);
        }
        std::make_heap(this->ordered_variables.begin(), this->ordered_variables.end(), Comparator{*this});
    }

    void VSIDSHeuristics::OnVariableActivity(Literal::Int variable) {
        auto &score = this->scores[variable - 1];
        score += ScoreIncrement;
        if (score > RescoreThreshold) {
            for (auto &score : this->scores) {
                score >>= RescoreFactor;
            }
        }

        std::make_heap(this->ordered_variables.begin(), this->ordered_variables.end(), Comparator{*this});
    }

    void VSIDSHeuristics::OnVariableAssignment(Literal::Int) {
        std::make_heap(this->ordered_variables.begin(), this->ordered_variables.end(), Comparator{*this});
    }

    Literal::Int VSIDSHeuristics::SuggestedVariableForAssignment() {
        if (this->ordered_variables.empty()) {
            return Literal::Terminator;
        } else {
            return this->ordered_variables.front();
        }
    }
}
