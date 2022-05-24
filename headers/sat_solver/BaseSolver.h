#ifndef SAT_SOLVER_BASE_SOLVER_H_
#define SAT_SOLVER_BASE_SOLVER_H_

#include "sat_solver/Core.h"
#include "sat_solver/Formula.h"
#include "sat_solver/Assignment.h"
#include "sat_solver/Watcher.h"
#include "sat_solver/DecisionTrail.h"
#include <algorithm>
#include <atomic>
#include <cassert>
#include <vector>

// This file contains base class for SAT solver implementation.
// The base solver contains common data structures (variable index, watchers, decision trail)
// that can be re-used by different SAT solving algorithms (e.g. DPLL, CDCL). Specific
// algorithms are expected to extend base class to benefit from it's functionality.

namespace sat_solver {

    template <typename C /* Needed for static polymorphism implementation via CRTP idiom */>
    class BaseSolver {
     public:
        inline const Formula &GetFormula() const {
            return this->formula;
        }

        inline const Assignment &GetAssignment() const {
            return this->assignment;
        }

        void Interrupt() {
            // Request current solving process to interrupt.
            // It's up to the specific algorithm implementation to check
            // the flag periodically.
            this->interrupt_requested = true;
        }

        void InterruptOn(std::function<bool()> req_fn) {
            this->interrupt_request_fn = std::move(req_fn);
        }

        SolverStatus Status() const {
            return this->current_status.load();
        }

        template <typename I>
        SolverStatus Solve(I assumptions_iter, I assumptions_end) {
            static_cast<C *>(this)->PreSolve();
            this->SaveAssumptions(std::move(assumptions_iter), std::move(assumptions_end));
            this->current_status = static_cast<C *>(this)->SolveImpl();
            static_cast<C *>(this)->PostSolve();
            return this->current_status;
        }

        SolverStatus Solve() {
            static_cast<C *>(this)->PreSolve();
            this->current_status = static_cast<C *>(this)->SolveImpl();
            static_cast<C *>(this)->PostSolve();
            return this->current_status;
        }

        template <typename T>
        friend class ModifiableSolverBase; // Modifiable solver needs access to clause attachment/detachment callbacks

     protected:
        enum class UnitPropagationResult {
            Sat, // Unit propagation resulted in a satisfied formula
            Unsat, // Unit propagation resulted in a conflict
            Pass // Unit propagation finished
        };

        enum class LiteralPolarity {
            PurePositive,
            PureNegative,
            Mixed,
            None
        };

        struct VariableIndexEntry {
            std::vector<std::size_t> positive_clauses{};
            std::vector<std::size_t> negative_clauses{};
            LiteralPolarity polarity{LiteralPolarity::None};
        };

        BaseSolver(const Formula &formula) // It is expected that the formula does not change without calling attachment/detachment callbacks
            : formula{formula},
            variable_index(formula.NumOfVariables(), VariableIndexEntry{}),
            assignment{formula.NumOfVariables()}, trail{formula.NumOfVariables()} {
            
            // Populate watcher list for the formula and update variable and clause indices for each clause
            for (std::size_t clause_idx = 0; clause_idx < this->formula.NumOfClauses(); clause_idx++) {
                const auto &clause = this->formula[clause_idx];
                this->watchers.emplace_back(clause);
                this->UpdateClauseIndex(clause_idx, clause);
            }
        }

        VariableIndexEntry &VariableIndex(Literal::UInt variable) {
            return this->variable_index[variable - 1];
        }

        void Assign(Literal::UInt variable, VariableAssignment assignment) {
            this->assignment[variable] = assignment;
            this->UpdateWatchers(variable);
            static_cast<C *>(this)->OnVariableAssignment(variable, assignment);
        }

        std::pair<UnitPropagationResult, std::size_t> UnitPropagation() {
            // Boolean constant propagation algorithm
            bool all_satisfied = false, propagate = true;
            while (propagate && !all_satisfied) { // Perform propagation at least once until there is nothing left to propagate or
                                                  // the formula is satisfied
                propagate = false;
                all_satisfied = true;
                for (std::size_t i = 0; !propagate && i < this->watchers.size(); i++) {
                    auto &watcher = this->watchers[i];

                    auto watcher_status = watcher.Status();

                    all_satisfied = all_satisfied && watcher_status == ClauseStatus::Satisfied;
                    if (watcher_status == ClauseStatus::Unit) {
                        auto watcher_unit = watcher.WatchedLiterals().first;
                        auto [variable, assignment] = this->formula[i][watcher_unit].Assignment();

                        this->trail.Propagation(variable, assignment, i);
                        this->Assign(variable, assignment);
                        propagate = true; // Do one more round of propagation
                    } else if (watcher_status == ClauseStatus::Unsatisfied) { // Unsatisfied clause found
                        return std::make_pair(UnitPropagationResult::Unsat, i);
                    }
                }
            }

            return std::make_pair(
                all_satisfied
                    ? UnitPropagationResult::Sat
                    : UnitPropagationResult::Pass,
                ClauseUndef);
        }

        void AttachClause(std::size_t clause_index, const ClauseView &clause) { // Callback that is called whenever a new clause is attached.
                                                                                // May be redefined in subclasses.
            this->ResetCurrentStatus();
            this->assignment.SetNumOfVariables(this->formula.NumOfVariables());
            this->trail.SetNumOfVariables(this->formula.NumOfVariables());
            if (this->formula.NumOfVariables() > this->variable_index.size()) { // If new variables were introduced, populate variable index with new entries
                this->variable_index.insert(this->variable_index.end(), this->formula.NumOfVariables() - this->variable_index.size(), VariableIndexEntry{});
            }

            // Create watcher for the new clause and update indices
            auto watcher_iter = this->watchers.emplace(this->watchers.begin() + clause_index, clause);
            this->UpdateClauseIndex(clause_index, clause);
            watcher_iter->Rescan(this->assignment);
        }

        void DetachClause(std::size_t index, const ClauseView &) { // Callback that is called whenever a new clause is detached.
                                                                   // May be redefined in subclasses.
            this->ResetCurrentStatus();
            this->assignment.SetNumOfVariables(this->formula.NumOfVariables());
            this->trail.SetNumOfVariables(this->formula.NumOfVariables());
            if (this->formula.NumOfVariables() < this->variable_index.size()) { // If number of variables has reduced, shrink variable index
                this->variable_index.erase(this->variable_index.begin() + this->formula.NumOfVariables(), this->variable_index.end());
            }

            // Update variable index to drop references to the removed clause
            for (auto &var_index : this->variable_index) {
                auto iterator = std::remove(var_index.positive_clauses.begin(), var_index.positive_clauses.end(), index);
                var_index.positive_clauses.erase(iterator, var_index.positive_clauses.end());
                iterator = std::remove(var_index.negative_clauses.begin(), var_index.negative_clauses.end(), index);
                var_index.negative_clauses.erase(iterator, var_index.negative_clauses.end());

                auto watcher_map = [index](auto watcher_index) {
                    if (watcher_index <= index) {
                        return watcher_index;
                    } else {
                        return watcher_index - 1;
                    }
                };
                std::transform(var_index.positive_clauses.begin(), var_index.positive_clauses.end(), var_index.positive_clauses.begin(), watcher_map);
                std::transform(var_index.negative_clauses.begin(), var_index.negative_clauses.end(), var_index.negative_clauses.begin(), watcher_map);
            }
            this->watchers.erase(this->watchers.begin() + index);
        }

        template <typename O>
        void ScanPureLiterals(O output) { // Go over variable index and write all pure literals into the output iterator
            for (Literal::UInt variable = 1; variable <= this->formula.NumOfVariables(); variable++) {
                auto polarity = this->VariableIndex(variable).polarity;
                if (this->assignment[variable] != VariableAssignment::Unassigned) {
                    continue;
                }

                switch (polarity) {
                    case LiteralPolarity::PurePositive:
                    case LiteralPolarity::None:
                        *(output++) = std::make_tuple(variable, VariableAssignment::True, false);
                        break;

                    case LiteralPolarity::PureNegative:
                        *(output++) = std::make_tuple(variable, VariableAssignment::False, false);
                        break;

                    case LiteralPolarity::Mixed:
                        // Intentionally left blank
                        break;
                }
            }
        }

        void OnVariableAssignment(Literal::UInt, VariableAssignment) {
            // Is called whenever the variable is assigned -- to be defined in the subclasses
        }

        void ResetState() { // Drop current assignment and decision trail
            this->pending_assignments.clear();
            this->assignment.Reset();
            this->trail.Reset();
            for (auto &watcher : this->watchers) {
                watcher.Rescan(this->assignment);
            }
        }

        bool PerformPendingAssignment(Literal::UInt variable, VariableAssignment variable_assignment, bool is_assumption) {
            // Perform assignment from pending list. Return value indicates whether assignment succeeded
            auto current_assignment = this->assignment[variable];
            if (is_assumption) { // Assumptions are enforced, failure to assign assumption is critical
                if (current_assignment == VariableAssignment::Unassigned) {
                    this->Assign(variable, variable_assignment);
                    this->trail.Assumption(variable, variable_assignment);
                } else if (current_assignment == variable_assignment) {
                    this->trail.Assumption(variable, variable_assignment);
                } else {
                    return false;
                }
            } else if (current_assignment == VariableAssignment::Unassigned) { // Other types of pending assignments are not enforced and
                                                                               // are omitted in case of conflict
                this->Assign(variable, variable_assignment);
                this->trail.Decision(variable, variable_assignment);
            }
            return true;
        }

        template <typename I>
        bool VerifyPendingAssignments(I iter, I end, Literal &conflict) { // Go over pending assignment range and check whether assumptions
                                                                          // are held in the current assignment. To be used when formula is satisfied
                                                                          // before all pending assignments are performed.
            for (; iter != end; std::advance(iter, 1)) {
                auto [variable, variable_assignment, is_assumption] = *iter;
                auto current_assignment = this->assignment[variable];
                if (is_assumption && current_assignment != VariableAssignment::Unassigned && current_assignment != variable_assignment) {
                    conflict = Literal{variable, variable_assignment};
                    return false;
                }
            }
            return true;
        }

        void PreSolve() { // Activities before solver starts
            if (!this->fresh_solver) {
                static_cast<C *>(this)->ResetState();
            }
            this->fresh_solver = false;
            this->interrupt_requested = false;
            this->current_status = SolverStatus::Solving;
        }

        void PostSolve() { // Activities after solver starts
#ifdef SAT_SOLVER_DEBUG_VALIDATIONS_ENABLE
            if (this->current_status == SolverStatus::Satisfied && !this->pending_assignments.empty()) {
                // For debugging purposes, check whether the model fulfills all assumptions
                for (auto [variable, variable_assignment, is_assumption] : this->pending_assignments) {
                    assert(!is_assumption || this->GetAssignment()[variable] != FlipVariableAssignment(variable_assignment));
                }
            }
#endif
        }

        template <typename I>
        void SaveAssumptions(I assumptions_iter, I assumptions_end) { // Translate range of assumptions into pending assignments
            for (; assumptions_iter != assumptions_end; std::advance(assumptions_iter, 1)) {
                auto literal = *assumptions_iter;
                auto [variable, var_assignment] = literal.Assignment();
                this->pending_assignments.emplace_back(variable, var_assignment, true);
            }
        }

        const Formula &formula;
        std::vector<VariableIndexEntry> variable_index{};
        std::vector<Watcher> watchers{};
        Assignment assignment;
        DecisionTrail trail;
        std::vector<std::tuple<Literal::UInt, VariableAssignment, bool>> pending_assignments{};
        std::atomic_bool interrupt_requested{false};
        std::atomic<SolverStatus> current_status{SolverStatus::Unknown};
        std::function<bool()> interrupt_request_fn{nullptr};

        static constexpr std::size_t ClauseUndef = ~static_cast<std::size_t>(0);

     private:
        void ResetCurrentStatus() {
            auto expected = this->current_status.load();
            do {
                switch (expected) {
                    case SolverStatus::Unknown:
                    case SolverStatus::Solving:
                        return;
                    
                    case SolverStatus::Satisfied:
                    case SolverStatus::Unsatisfied:
                        break;
                }
            } while (!this->current_status.compare_exchange_weak(expected, SolverStatus::Unknown));
        }

        void UpdateClauseIndex(std::size_t clause_idx, const ClauseView &clause) { // Scan literals from clause and update variable indices accordingly
            for (auto literal : clause) {
                auto &index_entry = this->VariableIndex(literal.Variable());
                auto &polarity = index_entry.polarity;
                if (literal.Assignment().second == VariableAssignment::True) {
                    index_entry.positive_clauses.emplace_back(clause_idx);

                    switch (polarity) {
                        case LiteralPolarity::PurePositive:
                        case LiteralPolarity::Mixed:
                            // Intentinally left blank
                            break;

                        case LiteralPolarity::PureNegative:
                            polarity = LiteralPolarity::Mixed;
                            break;

                        case LiteralPolarity::None:
                            polarity = LiteralPolarity::PurePositive;
                            break;
                    }
                } else {
                    index_entry.negative_clauses.emplace_back(clause_idx);

                    switch (polarity) {
                        case LiteralPolarity::PureNegative:
                        case LiteralPolarity::Mixed:
                            // Intentinally left blank
                            break;

                        case LiteralPolarity::PurePositive:
                            polarity = LiteralPolarity::Mixed;
                            break;

                        case LiteralPolarity::None:
                            polarity = LiteralPolarity::PureNegative;
                            break;
                    }
                }
            }
        }

        void UpdateWatchers(Literal::UInt variable) { // To be called whenever a variable gets assigned to update all affected watchers
            auto &var_index = this->VariableIndex(variable);
            for (auto affected_watcher : var_index.positive_clauses) {
                this->watchers[affected_watcher].Update(this->assignment, variable);
            }
            for (auto affected_watcher : var_index.negative_clauses) {
                this->watchers[affected_watcher].Update(this->assignment, variable);
            }
        }

        bool fresh_solver{true}; // Solver was freshly constructed -- to avoid full reset during the first call to solve
    };

    template <typename C>
    class ModifiableSolverBase { // Mixin to permit formula ownership and modification from within the solver
     public:
        const ClauseView &AppendClause(Clause clause) {
            const auto &view = this->owned_formula.AppendClause(std::move(clause));
            static_cast<C *>(this)->AttachClause(this->owned_formula.NumOfClauses() - 1, view);
            return view;
        }

        void RemoveClause(std::size_t index) {
            const auto &view = this->owned_formula[index];
            static_cast<C *>(this)->DetachClause(index, view);
            this->owned_formula.RemoveClause(index);
        }

     protected:
        ModifiableSolverBase(Formula formula)
            : owned_formula{std::move(formula)} {}

        Formula owned_formula;
    };
}

#endif
