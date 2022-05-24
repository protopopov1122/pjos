#ifdef SAT_SOLVER_IPASIR_INTERFACE_ENABLE

#ifdef SAT_SOLVER_IPASIR_INTERFACE_HEADER
#include SAT_SOLVER_IPASIR_INTERFACE_HEADER
#else
#include "ipasir.h"
#endif

#include "sat_solver/Solver.h"
#include <exception>
#include <iostream>
#include <vector>

#ifdef SAT_SOLVER_IPASIR_INTERFACE_ABORT_ON_ERROR
#define ABORT() std::abort()
#else
#define ABORT() ((void)0)
#endif

struct ipasir_solver {
    sat_solver::CdclSolver solver{};
    sat_solver::ClauseBuilder clause_builder{};
    std::vector<sat_solver::Literal> assumptions{};
    std::vector<sat_solver::Literal> final_conflict{};
};

IPASIR_API const char * ipasir_signature () {
    return sat_solver::CdclSolver::Signature().c_str();
}

IPASIR_API void * ipasir_init () {
    try {
        ipasir_solver *solver = new ipasir_solver;
        return solver;
    } catch (const std::exception &ex) {
        std::cerr << "ipasir_init: " << ex.what() << std::endl;
        ABORT();
    } catch (...) {
        std::cerr << "ipasir_init: caught an unknown exception" << std::endl;
        ABORT();
    }
}

IPASIR_API void ipasir_release (void * solver) {
    try {
        delete static_cast<ipasir_solver *>(solver);
    } catch (const std::exception &ex) {
        std::cerr << "ipasir_release: " << ex.what() << std::endl;
        ABORT();
    } catch (...) {
        std::cerr << "ipasir_release: caught an unknown exception" << std::endl;
        ABORT();
    }
}

IPASIR_API void ipasir_add (void * solver, int32_t lit_or_zero) {
    try {
        ipasir_solver *isolver = static_cast<ipasir_solver *>(solver);
        if (lit_or_zero != 0) {
            isolver->clause_builder.Add(lit_or_zero);
        } else {
            isolver->solver.AppendClause(isolver->clause_builder.Make());
        }
    } catch (const std::exception &ex) {
        std::cerr << "ipasir_add: " << ex.what() << std::endl;
        ABORT();
    } catch (...) {
        std::cerr << "ipasir_add: caught an unknown exception" << std::endl;
        ABORT();
    }
}

IPASIR_API void ipasir_assume (void * solver, int32_t lit) {
    try {
        ipasir_solver *isolver = static_cast<ipasir_solver *>(solver);
        isolver->assumptions.emplace_back(lit);
    } catch (const std::exception &ex) {
        std::cerr << "ipasir_assume: " << ex.what() << std::endl;
        ABORT();
    } catch (...) {
        std::cerr << "ipasir_assume: caught an unknown exception" << std::endl;
        ABORT();
    }
}

IPASIR_API int ipasir_solve (void * solver) {
    try {
        ipasir_solver *isolver = static_cast<ipasir_solver *>(solver);
        isolver->final_conflict.clear();
        auto res = isolver->solver.Solve(isolver->assumptions.begin(), isolver->assumptions.end(), std::back_inserter(isolver->final_conflict));
        isolver->assumptions.clear();
        switch (res) {
            case sat_solver::SolverStatus::Unknown:
            case sat_solver::SolverStatus::Solving:
                return 0;

            case sat_solver::SolverStatus::Satisfied:
                return 10;

            case sat_solver::SolverStatus::Unsatisfied:
                return 20;
        }
    } catch (const std::exception &ex) {
        std::cerr << "ipasir_solve: " << ex.what() << std::endl;
        ABORT();
    } catch (...) {
        std::cerr << "ipasir_solve: caught an unknown exception" << std::endl;
        ABORT();
    }
    return 0;
}

IPASIR_API int32_t ipasir_val (void * solver, int32_t lit) {
    try {
        ipasir_solver *isolver = static_cast<ipasir_solver *>(solver);
        auto assn = isolver->solver.GetAssignment().Of(lit);
        switch (assn) {
            case sat_solver::VariableAssignment::Unassigned:
                return 0;

            case sat_solver::VariableAssignment::True:
                return lit;

            case sat_solver::VariableAssignment::False:
                return -lit;
        }
    } catch (const std::exception &ex) {
        std::cerr << "ipasir_val: " << ex.what() << std::endl;
        ABORT();
    } catch (...) {
        std::cerr << "ipasir_val: caught an unknown exception" << std::endl;
        ABORT();
    }
    return 0;
}

IPASIR_API int ipasir_failed (void *solver, int32_t lit) {
    try {
        ipasir_solver *isolver = static_cast<ipasir_solver *>(solver);
        auto it = std::find(isolver->final_conflict.begin(), isolver->final_conflict.end(), lit);
        return it != isolver->final_conflict.end() ? 1 : 0;
    } catch (const std::exception &ex) {
        std::cerr << "ipasir_failed: " << ex.what() << std::endl;
        ABORT();
    } catch (...) {
        std::cerr << "ipasir_failed: caught an unknown exception" << std::endl;
        ABORT();
    }
}

IPASIR_API void ipasir_set_terminate (void *solver, void *data, int (*callback)(void *)) {
    try {
        ipasir_solver *isolver = static_cast<ipasir_solver *>(solver);
        if (callback != nullptr) {
            isolver->solver.InterruptOn([data, callback] {
                return callback(data) != 0;
            });
        } else {
            isolver->solver.InterruptOn(nullptr);
        }
    } catch (const std::exception &ex) {
        std::cerr << "ipasir_set_terminate: " << ex.what() << std::endl;
        ABORT();
    } catch (...) {
        std::cerr << "ipasir_set_terminate: caught an unknown exception" << std::endl;
        ABORT();
    }
}

IPASIR_API void ipasir_set_learn (void *, void *, int, void (*)(void *, int32_t *)) {
    std::cerr << "ipasir_set_learn: not implemented yet" << std::endl;
    ABORT();
}

#endif
