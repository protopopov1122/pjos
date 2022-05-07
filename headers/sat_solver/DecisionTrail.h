#ifndef SAT_SOLVER_DECISION_TRAIL_H_
#define SAT_SOLVER_DECISION_TRAIL_H_

#include "sat_solver/Assignment.h"
#include <functional>
#include <vector>

namespace sat_solver {

    class DecisionTrail {
        struct Entry {
            Entry(Literal::Int, VariableAssignment, bool, std::size_t);

            Literal::Int variable;
            VariableAssignment new_assignment;
            bool decision;
            std::size_t level;
        };

     public:
        DecisionTrail() = default;
        DecisionTrail(const DecisionTrail &) = default;
        DecisionTrail(DecisionTrail &&) = default;

        ~DecisionTrail() = default;

        DecisionTrail &operator=(const DecisionTrail &) = default;
        DecisionTrail &operator=(DecisionTrail &&) = default;

        inline std::size_t Level() const {
            return this->level;
        }

        void Decision(Literal::Int, VariableAssignment);
        void Propagation(Literal::Int, VariableAssignment);
        bool Undo(Literal::Int &, VariableAssignment &);

     private:
        std::vector<Entry> trail{};
        std::size_t level{0};
    };
}

#endif
