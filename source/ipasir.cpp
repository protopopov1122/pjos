#ifdef PJOS_IPASIR_INTERFACE_ENABLE

#ifdef PJOS_IPASIR_INTERFACE_HEADER
#include PJOS_IPASIR_INTERFACE_HEADER
#else
#include "ipasir.h"
#endif

#include "pjos/Solver.h"
#include <exception>
#include <iostream>
#include <vector>

#ifdef PJOS_IPASIR_INTERFACE_ABORT_ON_ERROR
#define ABORT() std::abort()
#else
#define ABORT() ((void)0)
#endif

struct ipasir_solver {
    pjos::CdclSolver solver{};
    pjos::ClauseBuilder clause_builder{};
    std::vector<pjos::Literal> assumptions{};
    std::vector<pjos::Literal> final_conflict{};
};

IPASIR_API const char * ipasir_signature () {
    return pjos::CdclSolver::Signature().c_str();
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
            case pjos::SolverStatus::Unknown:
            case pjos::SolverStatus::Solving:
                return 0;

            case pjos::SolverStatus::Satisfied:
                return 10;

            case pjos::SolverStatus::Unsatisfied:
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
            case pjos::VariableAssignment::Unassigned:
                return 0;

            case pjos::VariableAssignment::True:
                return lit;

            case pjos::VariableAssignment::False:
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

IPASIR_API void ipasir_set_learn (void *solver, void *data, int maxlen, void (*callback)(void *, int32_t *)) {
    try {
        ipasir_solver *isolver = static_cast<ipasir_solver *>(solver);
        if (callback != nullptr) {
            isolver->solver.OnLearnedClause([data, maxlen, callback](const auto &clause) mutable {
                if (static_cast<int>(clause.Length()) > maxlen) {
                    return;
                }

                constexpr std::size_t MaxStackAllocatedClauseLength = 255;
                if (clause.Length() > MaxStackAllocatedClauseLength) { 
                    std::vector<int32_t> content{};
                    for (auto literal : clause) {
                        content.push_back(literal.Get());
                    }
                    content.push_back(0);
                    callback(data, content.data());
                } else {
                    std::array<int32_t, MaxStackAllocatedClauseLength + 1> content;
                    auto it = content.begin();
                    for (auto literal : clause) {
                        *(it++) = literal.Get();
                    }
                    *(it++) = 0;
                    callback(data, content.data());
                }
            });
        } else {
            isolver->solver.OnLearnedClause(nullptr);
        }
    } catch (const std::exception &ex) {
        std::cerr << "ipasir_set_learn: " << ex.what() << std::endl;
        ABORT();
    } catch (...) {
        std::cerr << "ipasir_set_learn: caught an unknown exception" << std::endl;
        ABORT();
    }
}

#endif
