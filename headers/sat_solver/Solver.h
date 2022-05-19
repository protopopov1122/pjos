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
     public:
        inline const Formula &GetFormula() const {
            return this->formula;
        }

        inline const Assignment &GetAssignment() const {
            return this->assignment;
        }

        friend class ModifiableSolverBase;

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
        bool Backjump(std::size_t);

        const Formula &formula;
        std::unordered_map<Literal::Int, VariableIndexEntry> variable_index{};
        std::vector<Watcher> watchers{};
        Assignment assignment;
        DecisionTrail trail;

        static constexpr std::size_t ClauseUndef = ~static_cast<std::size_t>(0);
    };

    class ModifiableSolverBase {
     public:
        const ClauseView &AppendClause(Clause);

     protected:
        ModifiableSolverBase(BaseSolver &, Formula);

        BaseSolver &base_solver;
        Formula owned_formula;
    };

    class DpllSolver : public BaseSolver {
     public:
        DpllSolver(const Formula &);
        DpllSolver(const DpllSolver &) = default;
        DpllSolver(DpllSolver &&) = default;

        ~DpllSolver() = default;

        DpllSolver &operator=(const DpllSolver &) = default;
        DpllSolver &operator=(DpllSolver &&) = default;

        SolverStatus Solve();
    };

    class ModifiableDpllSolver : public ModifiableSolverBase, public DpllSolver {
     public:
        ModifiableDpllSolver(Formula);
        ModifiableDpllSolver(const ModifiableDpllSolver &) = default;
        ModifiableDpllSolver(ModifiableDpllSolver &&) = default;

        ~ModifiableDpllSolver() = default;

        ModifiableDpllSolver &operator=(const ModifiableDpllSolver &) = default;
        ModifiableDpllSolver &operator=(ModifiableDpllSolver &&) = default;
    };

    class CdclSolver : public ModifiableSolverBase, public BaseSolver {
     public:
        CdclSolver(Formula);
        CdclSolver(const CdclSolver &) = default;
        CdclSolver(CdclSolver &&) = default;

        ~CdclSolver() = default;

        CdclSolver &operator=(const CdclSolver &) = default;
        CdclSolver &operator=(CdclSolver &&) = default;

        SolverStatus Solve();

     private:
        std::pair<Clause, std::size_t> AnalyzeConflict(const ClauseView &);
        AnalysisTrackState &AnalysisTrackOf(Literal::Int);

        std::vector<AnalysisTrackState> analysis_track;
    };
}

#endif
