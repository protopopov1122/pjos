/*
    SPDX-License-Identifier: MIT
    SPDX-FileCopyrightText: 2022 Protopopovs Jevgenijs
*/

#ifndef PJOS_HEURISTICS_H_
#define PJOS_HEURISTICS_H_

#include "pjos/Formula.h"
#include "pjos/Assignment.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <unordered_set>

// Exponential variable state independent decaying sum (EVSIDS) heuristic
// keeps track of variable activity during the search and suggests the next
// variable to assign based on it's scoring. Exponential component of the heuristic
// means that on each iteration variable activity is valued more than the activity
// of previous iterations (rescaling of scores is performed automatically).

namespace pjos {

    class EVSIDSHeuristics {
     public:
        struct ScoringParameters {
            double rescore_threshold{1e100};
            double rescore_factor{1e-100};
            double initial_increment{1.0};
            double decay_rate{1.05};
        };

        EVSIDSHeuristics(const Formula &, const Assignment &, const ScoringParameters &);

        void Reset();
        void FormulaUpdated();
        void NextIteration();
        void VariableActive(Literal::UInt);
        void VariableAssigned(Literal::UInt);
        Literal::UInt PopVariable();

     private:
        struct Comparator {
            bool operator()(Literal::UInt, Literal::UInt) const;

            EVSIDSHeuristics &evsids;
        };

        const Formula &formula;
        const Assignment &assignment;
        ScoringParameters scoring;
        std::vector<double> scores;
        double score_increment{1.0};
        std::unordered_set<Literal::UInt> ordered_variable_index;
        std::vector<Literal::UInt> ordered_variables;
    };
}

#endif
