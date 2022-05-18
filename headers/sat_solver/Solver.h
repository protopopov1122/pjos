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

        enum class AnalysisTrackState {
            Untracked,
            Pending,
            Processed
        };

        BaseSolver(const Formula &);

        void Rebuild();
        void UpdateWatchers(Literal::Int);
        void Assign(Literal::Int, VariableAssignment);
        std::pair<UnitPropagationResult, std::size_t> UnitPropagation();

        const Formula &formula;
        std::unordered_map<Literal::Int, VariableIndexEntry> variable_index{};
        std::vector<Watcher> watchers{};
        Assignment assignment;
        DecisionTrail trail;

        static constexpr std::size_t ClauseUndef = ~static_cast<std::size_t>(0);
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

    class CdclSolver : public BaseSolver {
     public:
        CdclSolver(const Formula &);
        CdclSolver(const CdclSolver &) = default;
        CdclSolver(CdclSolver &&) = default;

        ~CdclSolver() = default;

        CdclSolver &operator=(const CdclSolver &) = default;
        CdclSolver &operator=(CdclSolver &&) = default;

        inline const Assignment &GetAssignment() const {
            return this->assignment;
        }

        SolverStatus Solve();

     private:
        Clause AnalyzeConflict(const ClauseView &);
        AnalysisTrackState &AnalysisTrackOf(Literal::Int);
        
        std::vector<AnalysisTrackState> analysis_track;
    };
}

#endif
