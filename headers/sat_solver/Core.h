#ifndef SAT_SOLVER_CORE_H_
#define SAT_SOLVER_CORE_H_

#include "sat_solver/Base.h"

#define SAT_SOLVER_IDENTIFIER "SAT Solver"
#define SAT_SOLVER_VERSION "v0.0.1"

namespace sat_solver {

    enum class VariableAssignment {
        Unassigned = -1,
        False = 0,
        True = 1
    };

    enum class SolverStatus {
        Unsatisfied,
        Satisfied,
        Unknown,
        Solving
    };
}

#endif
