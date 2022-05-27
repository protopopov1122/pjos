/*
    SPDX-License-Identifier: MIT
    SPDX-FileCopyrightText: 2022 Protopopovs Jevgenijs
*/

#ifndef PJOS_CORE_H_
#define PJOS_CORE_H_

#include "pjos/Base.h"

#define PJOS_IDENTIFIER "PJOS SAT Solver"
#define PJOS_VERSION "v0.0.1"

namespace pjos {

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
