#ifndef PJOS_DPLL_SOLVER_H_
#define PJOS_DPLL_SOLVER_H_

#include "pjos/BaseSolver.h"

// This file contains specific implementation of SAT solvers for DPLL algorithm

namespace pjos {

    class DpllSolver : public BaseSolver<DpllSolver> {
     public:
        DpllSolver(const Formula &);
        DpllSolver(const DpllSolver &) = default;
        DpllSolver(DpllSolver &&) = default;

        ~DpllSolver() = default;

        DpllSolver &operator=(const DpllSolver &) = default;
        DpllSolver &operator=(DpllSolver &&) = default;

        static const std::string &Signature();

        friend class BaseSolver<DpllSolver>; // Necessary for static polymorphism implementation

     private:
        SolverStatus SolveImpl();
    };

    class ModifiableDpllSolver : public ModifiableSolverBase<ModifiableDpllSolver>, public DpllSolver {
     public:
        ModifiableDpllSolver();
        ModifiableDpllSolver(Formula);
        ModifiableDpllSolver(const ModifiableDpllSolver &) = default;
        ModifiableDpllSolver(ModifiableDpllSolver &&) = default;

        ~ModifiableDpllSolver() = default;

        ModifiableDpllSolver &operator=(const ModifiableDpllSolver &) = default;
        ModifiableDpllSolver &operator=(ModifiableDpllSolver &&) = default;
    };
}

#endif
