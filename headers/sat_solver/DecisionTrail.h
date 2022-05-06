#ifndef SAT_SOLVER_DECISION_TRAIL_H_
#define SAT_SOLVER_DECISION_TRAIL_H_

#include "sat_solver/Assignment.h"
#include <functional>
#include <vector>

namespace sat_solver {

    class DecisionTrail {
        struct Entry {
            Entry(Literal::Int, VariableAssignment, VariableAssignment, bool, std::size_t);

            Literal::Int variable;
            VariableAssignment new_assignment;
            VariableAssignment prev_assignment;
            bool decision;
            std::size_t level;
        };

     public:
        DecisionTrail(Assignment &, std::function<void(Literal::Int, VariableAssignment)>);
        DecisionTrail(const DecisionTrail &) = default;
        DecisionTrail(DecisionTrail &&) = default;

        ~DecisionTrail() = default;

        DecisionTrail &operator=(const DecisionTrail &) = delete;
        DecisionTrail &operator=(DecisionTrail &&) = delete;

        inline std::size_t Level() const {
            return this->level;
        }

        void Decision(Literal::Int, VariableAssignment);
        void Propagation(Literal::Int, VariableAssignment);
        std::pair<Literal::Int, VariableAssignment> UndoDecision();

     private:
        Assignment &assignment;
        std::function<void(Literal::Int, VariableAssignment)> assign_fn;
        std::vector<Entry> trail{};
        std::size_t level;
    };
}

#endif
