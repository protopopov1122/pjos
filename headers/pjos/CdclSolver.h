#ifndef PJOS_CDCL_SOLVER_H_
#define PJOS_CDCL_SOLVER_H_

#include "pjos/BaseSolver.h"
#include "pjos/Heuristics.h"
#include <algorithm>
#include <set>
#include <vector>

// This file contains specific implementation of SAT solver for CDCL algorithm

namespace pjos {

    class CdclSolver : public ModifiableSolverBase<CdclSolver>, public BaseSolver<CdclSolver> {
     public:
        CdclSolver();
        CdclSolver(Formula);
        CdclSolver(const CdclSolver &) = default;
        CdclSolver(CdclSolver &&) = default;

        ~CdclSolver() = default;

        CdclSolver &operator=(const CdclSolver &) = default;
        CdclSolver &operator=(CdclSolver &&) = default;

        void OnLearnedClause(std::function<void(const ClauseView &)>);

        using BaseSolver<CdclSolver>::Solve; // Import existing solve methods

        template <typename I, typename O>
        SolverStatus Solve(I assumptions_iter, I assumptions_end, O final_conflict_output) { // Solve and output the final conflict -- set of assumptions
                                                                                             // that led to unsatisfied formula
            this->PreSolve();
            this->SaveAssumptions(std::move(assumptions_iter), std::move(assumptions_end));
            this->current_status = this->SolveImpl(true);
            this->PostSolve();
            
            if (this->current_status == SolverStatus::Unsatisfied) {
                std::copy(this->final_conflict.begin(), this->final_conflict.end(), std::move(final_conflict_output));
                this->final_conflict.clear();
            }
            return this->current_status;
        }

        static const std::string &Signature();

        friend class ModifiableSolverBase<CdclSolver>; // Necessary for static polymorphism implementation
        friend class BaseSolver<CdclSolver>;

     private:
        enum class AnalysisTrackState { // Markings for variable assignments in decision trail during the analysis stage
            Untracked, // Assignment is not involved in the analysis graph at the moment
            Pending,   // Assignment was seen by the analyzer and to be inspected later
            Processed  // Assignment was seen and inspected by the analyzer
        };

        struct VariableOccurences { // Calculate total number of variable occurences in the formula
            std::size_t operator()(Literal::UInt) const;

            CdclSolver &solver;
        };

        SolverStatus SolveImpl(bool = false);
        bool Backjump(std::size_t);
        void AttachClause(std::size_t, const ClauseView &);
        void DetachClause(std::size_t, const ClauseView &);
        void OnVariableAssignment(Literal::UInt, VariableAssignment);

        std::pair<Clause, std::size_t> AnalyzeConflict(const ClauseView &);
        AnalysisTrackState &AnalysisTrackOf(Literal::UInt);
        
        template <typename T>
        void AnalyzeFinalConflict(T conflict_clause_iter, T conflict_clause_end, bool assumption_clause) { // Analyze final conflict and derive a set of assumptions
                                                                                                           // that led to the conflict
            std::fill(this->analysis_track.begin(), this->analysis_track.end(), AnalysisTrackState::Untracked);

            std::size_t pending = this->MarkClauseForFinalConflictAnalysis(conflict_clause_iter, conflict_clause_end, assumption_clause); // Number of assignments in pending state
            std::size_t trail_index = this->trail.Length() - 1; // Scan trail from the end to the beginning
            while (pending > 0) {
                while (this->AnalysisTrackOf(this->trail[trail_index].variable) != AnalysisTrackState::Pending) { // Find next pending assignment for analysis
                    assert(trail_index > 0);
                    trail_index--;
                }

                const auto &trail_entry = this->trail[trail_index];
                this->AnalysisTrackOf(trail_entry.variable) = AnalysisTrackState::Processed;
                pending--;

                if (DecisionTrail::IsPropagatedFromClause(trail_entry.reason)) { // The assignment is a result of propagation. Scan corresponding clause for further analysis
                    const auto &clause = this->formula[trail_entry.reason];
                    pending += this->MarkClauseForFinalConflictAnalysis(clause.begin(), clause.end(), false);
                } else if (trail_entry.reason == DecisionTrail::ReasonAssumption) { // The assignment is a result of assumption, put the assumption into the final conflict
                    this->final_conflict.insert(Literal{trail_entry.variable, trail_entry.assignment});
                }
            }
        }

        template <typename I>
        std::size_t MarkClauseForFinalConflictAnalysis(I conflict_clause_iter, I conflict_clause_end, bool assumption_clause) { // Scan a clause and mark all corresponding assignments as pending for analysis
            std::size_t pending{0};
            for (; conflict_clause_iter != conflict_clause_end; std::advance(conflict_clause_iter, 1)) {
                auto literal = *conflict_clause_iter;
                auto variable = literal.Variable();
                if (this->AnalysisTrackOf(variable) != AnalysisTrackState::Untracked) { // If assignment was already seen, skip it
                    continue;
                }

                auto trail_entry = this->trail.Find(variable);
                assert(trail_entry != nullptr);
                if (DecisionTrail::IsPropagatedFromClause(trail_entry->reason) && !assumption_clause) { // If the assignment is a result of propagation, mark it for further analysis
                    this->AnalysisTrackOf(variable) = AnalysisTrackState::Pending;
                    pending++;
                } else if (trail_entry->reason == DecisionTrail::ReasonAssumption || assumption_clause) { // If the assignment is an assumption itself, put in into the final conflict
                    this->final_conflict.insert(Literal{trail_entry->variable, trail_entry->assignment});
                }
            }
            return pending;
        }

        std::vector<AnalysisTrackState> analysis_track;
        EVSIDSHeuristics<VariableOccurences> evsids;
        Assignment saved_phases;
        std::set<Literal> final_conflict;
        std::function<void(const ClauseView &)> learned_clause_fn;
    };
}

#endif
