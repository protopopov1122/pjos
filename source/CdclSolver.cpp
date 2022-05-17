#include "sat_solver/Solver.h"

namespace sat_solver {

    CdclSolver::CdclSolver(const Formula &formula)
        : BaseSolver::BaseSolver{formula} {}

    SolverStatus CdclSolver::Solve() {
        return SolverStatus::Unsatisfied;
    }
}