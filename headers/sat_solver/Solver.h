#ifndef SAT_SOLVER_SOLVER_H_
#define SAT_SOLVER_SOLVER_H_

#include "sat_solver/BaseSolver.h"
#include "sat_solver/Heuristics.h"
#include <algorithm>
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

        using BaseSolver<CdclSolver>::Solve;

        template <typename I, typename O>
        SolverStatus Solve(I assumptions_iter, I assumptions_end, O final_conflict_output) {
            this->analyze_final_conflict = true;
            auto result = this->Solve(std::move(assumptions_iter), std::move(assumptions_end));
            if (result == SolverStatus::Unsatisfied) {
                std::copy(this->final_conflict.begin(), this->final_conflict.end(), std::move(final_conflict_output));
                this->final_conflict.clear();
            }
            return result;
        }

        static const std::string &Signature();

        friend class ModifiableSolverBase<CdclSolver>;
        friend class BaseSolver<CdclSolver>;

     private:
        struct VariableOccurences {
            std::size_t operator()(Literal::UInt) const;

            CdclSolver &solver;
        };

        SolverStatus SolveImpl();
        bool Backjump(std::size_t);
        void AttachClause(std::size_t, const ClauseView &);
        void DetachClause(std::size_t, const ClauseView &);
        void OnVariableAssignment(Literal::UInt, VariableAssignment);

        std::pair<Clause, std::size_t> AnalyzeConflict(const ClauseView &);
        AnalysisTrackState &AnalysisTrackOf(Literal::UInt);
        
        template <typename T>
        void AnalyzeFinalConflict(T conflict_clause_iter, T conflict_clause_end, bool assumption_clause) {
            std::fill(this->analysis_track.begin(), this->analysis_track.end(), AnalysisTrackState::Untracked);

            std::size_t pending = this->MarkClauseForFinalConflictAnalysis(conflict_clause_iter, conflict_clause_end, assumption_clause);
            std::size_t trail_index = this->trail.Length() - 1;
            while (pending > 0) {
                while (this->AnalysisTrackOf(this->trail[trail_index].variable) != AnalysisTrackState::Pending) {
                    assert(trail_index > 0);
                    trail_index--;
                }

                const auto &trail_entry = this->trail[trail_index];
                this->AnalysisTrackOf(trail_entry.variable) = AnalysisTrackState::Processed;
                pending--;

                if (DecisionTrail::IsPropagatedFromClause(trail_entry.reason)) {
                    const auto &clause = this->formula[trail_entry.reason];
                    pending += this->MarkClauseForFinalConflictAnalysis(clause.begin(), clause.end(), false);
                } else if (trail_entry.reason == DecisionTrail::ReasonAssumption) {
                    this->final_conflict.push_back(Literal{trail_entry.variable, trail_entry.assignment});
                }
            }
        }

        template <typename I>
        std::size_t MarkClauseForFinalConflictAnalysis(I conflict_clause_iter, I conflict_clause_end, bool assumption_clause) {
            std::size_t pending{0};
            for (; conflict_clause_iter != conflict_clause_end; std::advance(conflict_clause_iter, 1)) {
                auto literal = *conflict_clause_iter;
                auto variable = literal.Variable();
                if (this->AnalysisTrackOf(variable) != AnalysisTrackState::Untracked) {
                    continue;
                }

                auto trail_entry = this->trail.Find(variable);
                assert(trail_entry != nullptr);
                if (DecisionTrail::IsPropagatedFromClause(trail_entry->reason) && !assumption_clause) {
                    this->AnalysisTrackOf(variable) = AnalysisTrackState::Pending;
                    pending++;
                } else if (trail_entry->reason == DecisionTrail::ReasonAssumption || assumption_clause) {
                    this->final_conflict.push_back(Literal{trail_entry->variable, trail_entry->assignment});
                }
            }
            return pending;
        }

        std::vector<AnalysisTrackState> analysis_track;
        VSIDSHeuristics<VariableOccurences, 1, 1024, 1> vsids;
        bool analyze_final_conflict;
        std::vector<Literal> final_conflict;
    };
}

#endif
