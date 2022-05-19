#ifndef SAT_SOLVER_HEURISTICS_H_
#define SAT_SOLVER_HEURISTICS_H_

#include "sat_solver/Formula.h"
#include "sat_solver/Assignment.h"

namespace sat_solver {

    class VSIDSHeuristics {
     public:
        VSIDSHeuristics(const Formula &, const Assignment &);

        void Reset();
        void OnUpdate();
        void OnVariableActivity(Literal::Int);
        void OnVariableAssignment(Literal::Int);
        Literal::Int SuggestedVariableForAssignment();

     private:
        struct Comparator {
            bool operator()(Literal::Int, Literal::Int) const;

            VSIDSHeuristics &vsids;
        };

        const Formula &formula;
        const Assignment &assignment;
        std::vector<std::int64_t> scores;
        std::vector<Literal::Int> ordered_variables;
    };
}

#endif
