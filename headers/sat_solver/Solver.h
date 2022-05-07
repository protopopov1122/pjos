#ifndef SAT_SOLVER_SOLVER_H_
#define SAT_SOLVER_SOLVER_H_

#include "sat_solver/Core.h"
#include "sat_solver/Formula.h"
#include "sat_solver/Assignment.h"
#include "sat_solver/Watcher.h"
#include "sat_solver/DecisionTrail.h"
#include <unordered_map>
#include <vector>

namespace sat_solver {

    namespace internal {
        
        struct VariableIndexEntry {
            std::vector<std::size_t> positive_clauses;
            std::vector<std::size_t> negative_clauses;
        };

        struct SolverState {
            SolverState(const Formula &);

            void Rebuild();
            void UpdateWatchers(Literal::Int);
            void Assign(Literal::Int, VariableAssignment);

            const Formula &formula;
            std::unordered_map<Literal::Int, VariableIndexEntry> variable_index{};
            std::vector<Watcher> watchers{};
            Assignment assignment;
            DecisionTrail trail{};
        };
    }

    class DpllSolver {
     public:
        DpllSolver(const Formula &);
        DpllSolver(const DpllSolver &) = default;
        DpllSolver(DpllSolver &&) = default;

        ~DpllSolver() = default;

        DpllSolver &operator=(const DpllSolver &) = default;
        DpllSolver &operator=(DpllSolver &&) = default;

        inline const Assignment &GetAssignment() const {
            return this->state.assignment;
        }

        SolverStatus Solve();

     private:
        enum class BcpResult {
            Sat,
            Unsat,
            Pass
        };

        BcpResult Bcp();

        internal::SolverState state;
    };
}

#endif
