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

    class BaseSolver {
     protected:
        struct VariableIndexEntry {
            std::vector<std::size_t> positive_clauses;
            std::vector<std::size_t> negative_clauses;
        };

        enum class UnitPropagationResult {
            Sat,
            Unsat,
            Pass
        };

        BaseSolver(const Formula &);

        void Rebuild();
        void UpdateWatchers(Literal::Int);
        void Assign(Literal::Int, VariableAssignment);
        UnitPropagationResult UnitPropagation();

        const Formula &formula;
        std::unordered_map<Literal::Int, VariableIndexEntry> variable_index{};
        std::vector<Watcher> watchers{};
        Assignment assignment;
        DecisionTrail trail{};
    };

    class DpllSolver : public BaseSolver {
     public:
        DpllSolver(const Formula &);
        DpllSolver(const DpllSolver &) = default;
        DpllSolver(DpllSolver &&) = default;

        ~DpllSolver() = default;

        DpllSolver &operator=(const DpllSolver &) = default;
        DpllSolver &operator=(DpllSolver &&) = default;

        inline const Assignment &GetAssignment() const {
            return this->assignment;
        }

        SolverStatus Solve();
    };
}

#endif
