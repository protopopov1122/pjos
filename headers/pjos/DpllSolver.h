/*
    SPDX-License-Identifier: MIT
    SPDX-FileCopyrightText: 2022 Protopopovs Jevgenijs
*/

#ifndef PJOS_DPLL_SOLVER_H_
#define PJOS_DPLL_SOLVER_H_

#include "pjos/BaseSolver.h"

// Simple SAT solver implementing DPLL algorithm.
// Solver has most optimizations turned off and is inteded to be
// used as a baseline which contains (hopefully) less bugs than
// more sophisticated solvers. DPLL solver is available in two
// variants which differ in whether the underlying formula is intended to be
// modified after solver construction.

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
