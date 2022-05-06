#ifndef SAT_SOLVER_CORE_H_
#define SAT_SOLVER_CORE_H_

#include "sat_solver/Base.h"

namespace sat_solver {

    enum class LiteralPolarity {
        Negative = -1,
        Positive = 1
    };

    enum class VariableAssignment {
        Unassigned = 100,
        False = 0,
        True = 1
    };

    enum class SolverStatus {
        Unsatisfiable,
        Satisfiable
    };
}

#endif
