#ifndef SAT_SOLVER_SOLVER_H_
#define SAT_SOLVER_SOLVER_H_

#include "sat_solver/BaseSolver.h"
#include "sat_solver/Heuristics.h"
#include <vector>

namespace sat_solver {

    class DpllSolver : public BaseSolver<DpllSolver> {
     public:
        DpllSolver(const Formula &);
        DpllSolver(const DpllSolver &) = default;
        DpllSolver(DpllSolver &&) = default;

        ~DpllSolver() = default;

        DpllSolver &operator=(const DpllSolver &) = default;
        DpllSolver &operator=(DpllSolver &&) = default;

        static const std::string &Signature();

        friend class BaseSolver<DpllSolver>;

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

    class CdclSolver : public ModifiableSolverBase<CdclSolver>, public BaseSolver<CdclSolver> {
     public:
        CdclSolver();
        CdclSolver(Formula);
        CdclSolver(const CdclSolver &) = default;
        CdclSolver(CdclSolver &&) = default;

        ~CdclSolver() = default;

        CdclSolver &operator=(const CdclSolver &) = default;
        CdclSolver &operator=(CdclSolver &&) = default;

        static const std::string &Signature();

        friend class ModifiableSolverBase<CdclSolver>;
        friend class BaseSolver<CdclSolver>;

     private:
        struct VariableOccurences {
            std::size_t operator()(Literal::Int) const;

            CdclSolver &solver;
        };

        SolverStatus SolveImpl();
        bool Backjump(std::size_t);
        void AttachClause(std::size_t, const ClauseView &);
        void DetachClause(std::size_t, const ClauseView &);
        void OnVariableAssignment(Literal::Int, VariableAssignment);

        std::pair<Clause, std::size_t> AnalyzeConflict(const ClauseView &);
        AnalysisTrackState &AnalysisTrackOf(Literal::Int);

        std::vector<AnalysisTrackState> analysis_track;
        VSIDSHeuristics<VariableOccurences, 1, 1024, 1> vsids;
    };
}

#endif
