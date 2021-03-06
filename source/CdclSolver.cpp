/*
    SPDX-License-Identifier: MIT
    SPDX-FileCopyrightText: 2022 Protopopovs Jevgenijs
*/

#include "pjos/CdclSolver.h"
#include <algorithm>
#include <cassert>
#include <limits>

// This file contains implementation of CDCL-based SAT solver with some heuristics.

namespace pjos {

    CdclSolver::CdclSolver(const Heuristics::ScoringParameters &scoring)
        : CdclSolver::CdclSolver(Formula{}, scoring) {}

    CdclSolver::~CdclSolver() {
        this->owned_formula.Clear();
    }

    CdclSolver::CdclSolver(Formula formula, const Heuristics::ScoringParameters &scoring)
        : ModifiableSolverBase::ModifiableSolverBase(std::move(formula)),
          BaseSolver::BaseSolver{this->owned_formula},
          analysis_track(formula.NumOfVariables(), AnalysisTrackState::Untracked),
          evsids{this->formula, this->assignment, scoring},
          saved_phases{this->formula.NumOfVariables()} {}

    void CdclSolver::OnLearnedClause(std::function<void(const ClauseView &)> fn) {
        this->learned_clause_fn = std::move(fn);
    }

    const std::string &CdclSolver::Signature() {
        static std::string sig{PJOS_IDENTIFIER " (CDCL) " PJOS_VERSION};
        return sig;
    }

    SolverStatus CdclSolver::SolveImpl(bool analyze_final_conflict) {
        if (this->parameters.pure_literal_elimination) {
            this->ScanPureLiterals(std::back_inserter(this->pending_assignments)); // Find pure literals to assign those first
        }

        auto pending_assignments_iter = this->pending_assignments.begin();
        std::size_t number_of_assumptions{0};
        for (;;) {
            if (this->interrupt_requested.load() || (this->interrupt_request_fn != nullptr && this->interrupt_request_fn())) {
                return SolverStatus::Unknown;
            }

            auto [bcp_result, conflict_clause] = this->UnitPropagation();
            if (bcp_result == UnitPropagationResult::Sat) { // BCP satisfied the formula. Check whether all assumptions are also satisfied.
                Literal conflict[2];
                if (this->VerifyPendingAssignments(pending_assignments_iter, this->pending_assignments.end(), conflict[0])) {
                    return SolverStatus::Satisfied;
                } else { // Conflicting assumption was found. Build final conflict and return UNSAT
                    if (analyze_final_conflict) {
                        this->AnalyzeFinalConflict(&conflict[0], &conflict[1], true);
                    }
                    return SolverStatus::Unsatisfied;
                }
            } else if (bcp_result == UnitPropagationResult::Unsat) { // BCP resulted in a conflict
                if (this->trail.Level() == 0) { // There were no decisions/assumptions made, formula is UNSAT
                    if (analyze_final_conflict) {
                        const auto &conflict = this->formula[conflict_clause];
                        this->AnalyzeFinalConflict(conflict.begin(), conflict.end(), false);
                    }
                    return SolverStatus::Unsatisfied;
                }

                // Analyze the conflict, obtain learned clause and non-chronological backjump level
                auto [learned_clause, backjump_level] = this->AnalyzeConflict(this->formula[conflict_clause]);
                this->AppendClause(std::move(learned_clause));
                if (this->learned_clause_fn != nullptr) {
                    this->learned_clause_fn(learned_clause);
                }
                
                // Try to backjump. If the backjump fails or involves assumptions, the formula is UNSAT.
                if (backjump_level < number_of_assumptions || !this->Backjump(backjump_level)) {
                    if (analyze_final_conflict) {
                        const auto &conflict = this->formula[conflict_clause];
                        this->AnalyzeFinalConflict(conflict.begin(), conflict.end(), false);
                    }
                    return SolverStatus::Unsatisfied;
                }

                this->evsids.NextIteration();
            } else if (pending_assignments_iter == this->pending_assignments.end()) { // There are no pending assignments. Select variable for further assignment.
                Literal::UInt variable = this->evsids.PopVariable(); // The most active unassigned variable
                assert(variable != Literal::Terminator);
                assert(this->assignment[variable] == VariableAssignment::Unassigned);

                VariableAssignment variable_assignment;
                if (this->parameters.phase_saving && this->saved_phases[variable] != VariableAssignment::Unassigned) {
                    variable_assignment = this->saved_phases[variable];
                } else {
                    variable_assignment = VariableAssignment::True;
                }
                this->trail.Decision(variable, variable_assignment);
                this->Assign(variable, variable_assignment);
            } else { // There is pending assignment, peform it
                auto [variable, variable_assignment, is_assumption] = *pending_assignments_iter;
                std::advance(pending_assignments_iter, 1);
                if (!this->PerformPendingAssignment(variable, variable_assignment, is_assumption)) { // Pending assignment failed, analyze final conflict
                    if (analyze_final_conflict) {
                        Literal conflict[2] = {Literal{variable, variable_assignment}};
                        this->AnalyzeFinalConflict(&conflict[0], &conflict[1], true);
                    }
                    return SolverStatus::Unsatisfied;
                }
                if (is_assumption) { // Another assumption was made
                    number_of_assumptions++;
                }
            }
        }
    }

    std::pair<Clause, std::size_t> CdclSolver::AnalyzeConflict(const ClauseView &conflict) { // Conflict analysis and clause learning algorithm
        assert(this->trail.Level() > 0);
        std::fill(this->analysis_track.begin(), this->analysis_track.end(), AnalysisTrackState::Untracked); // Clean up assignment tracking information

        // Breadth-first search in a subgraph of the decision graph that is delimited by
        // UIP cut. That is - only nodes that are not below current decision level are considered
        // and the search stops once UIP if found
        const ClauseView *clause = std::addressof(conflict); // Clause that is currently considered
        std::size_t trail_index = this->trail.Length() - 1; // Scan the trail from the end to the beginning
        std::size_t number_of_paths = 1; // Number of paths under BFS consideration at the moment. Once UIP is found, the number will converge to 1
        std::size_t backjump_level = 0;
        do {
            for (auto literal : *clause) { // Enumerate literals in the current clause
                auto variable = literal.Variable();
                if (this->AnalysisTrackOf(variable) != AnalysisTrackState::Untracked) { // Skip those that were already seen
                    continue;
                }

                auto trail_entry = this->trail.Find(variable);
                assert(trail_entry != nullptr);
                if (trail_entry->level >= this->trail.Level()) { // The assignment is part of analyzed subgraph, mark it for further analysis
                    this->AnalysisTrackOf(variable) = AnalysisTrackState::Pending;
                    number_of_paths++;
                } else { // The assignment is part of UIP cut, add it to learned clause
                    learned_clause.Add(Literal{variable, FlipVariableAssignment(trail_entry->assignment)});
                    backjump_level = std::max(backjump_level, trail_entry->level);
                }
                this->evsids.VariableActive(variable);
            }
            number_of_paths--;

            while (this->AnalysisTrackOf(this->trail[trail_index].variable) != AnalysisTrackState::Pending) { // Find next assignment for analysis
                assert(trail_index > 0);
                trail_index--;
            }
            assert(this->AnalysisTrackOf(this->trail[trail_index].variable) == AnalysisTrackState::Pending);
            this->AnalysisTrackOf(this->trail[trail_index].variable) = AnalysisTrackState::Processed;
            const auto &trail_entry = this->trail[trail_index];
            if (DecisionTrail::IsPropagatedFromClause(trail_entry.reason)) { // The assignment is a result of propagation
                clause = std::addressof(this->formula[trail_entry.reason]);
            } else { // The assignment is a decision/assumption -- it shall be the UIP
                assert(number_of_paths == 1);
            }
        } while (number_of_paths > 1);

        // Add the top-most decision/assumption to the learned clause
        const auto &trail_entry = this->trail[trail_index];
        learned_clause.Add(Literal{trail_entry.variable, FlipVariableAssignment(trail_entry.assignment)});
        this->evsids.VariableActive(trail_entry.variable);
        assert(trail_entry.level > 0);
        if (backjump_level == 0) { // If no backjump level was found, jump to the one below current
            backjump_level = std::max(backjump_level, trail_entry.level - 1);
        }
        assert(trail_entry.level == this->trail.Level());
        assert(backjump_level < this->trail.Level());

        return std::make_pair(learned_clause.Make(this->literal_allocator), backjump_level);
    }

    CdclSolver::AnalysisTrackState &CdclSolver::AnalysisTrackOf(Literal::UInt variable) {
        return this->analysis_track[variable - 1];
    }

    bool CdclSolver::Backjump(std::size_t level) { // Perform backjump to the level -- undo all the assignments done after that level was reached
        while (this->trail.Level() > level) {
            auto trail_entry = this->trail.Top();
            if (trail_entry == nullptr) { // There is nothing to backjump to -- failure
                return false;
            }

            assert(trail_entry->reason != DecisionTrail::ReasonAssumption);

            if (this->parameters.phase_saving && trail_entry->reason == DecisionTrail::ReasonDecision && trail_entry->level > level) {
                // Decisions above current backjump level are not causes of the conflict, thus
                // might be saved and reused later
                this->saved_phases[trail_entry->variable] = trail_entry->assignment;
            }

            this->Assign(trail_entry->variable, VariableAssignment::Unassigned);
            this->trail.Pop();
        }
        return true;
    }

    void CdclSolver::AttachClause(std::size_t clause_index, const ClauseView &clause) { // Whenever the clause is attached, update analysis track and heuristics
        this->BaseSolver<CdclSolver>::AttachClause(clause_index, clause);

        if (this->formula.NumOfVariables() > this->analysis_track.size()) {
            this->analysis_track.insert(this->analysis_track.end(), this->formula.NumOfVariables() - this->analysis_track.size(), AnalysisTrackState::Untracked);
        }
        this->evsids.FormulaUpdated();
        this->saved_phases.SetNumOfVariables(this->formula.NumOfVariables());
    }

    void CdclSolver::DetachClause(std::size_t clause_index, const ClauseView &clause) { // Whenever the clause is detached, update analysis track and heuristics
        this->BaseSolver<CdclSolver>::DetachClause(clause_index, clause);

        if (this->formula.NumOfVariables() < this->analysis_track.size()) {
            this->analysis_track.erase(this->analysis_track.begin() + this->formula.NumOfVariables(), this->analysis_track.end());
        }
        this->evsids.FormulaUpdated();
        this->saved_phases.SetNumOfVariables(this->formula.NumOfVariables());
    }

    void CdclSolver::OnVariableAssignment(Literal::UInt variable, VariableAssignment) { // Assignment was performed, update heuristics
        this->evsids.VariableAssigned(variable);
    }
}