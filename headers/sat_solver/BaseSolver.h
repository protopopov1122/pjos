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

namespace sat_solver {

    template <typename C>
    class BaseSolver {
     public:
        inline const Formula &GetFormula() const {
            return this->formula;
        }

        inline const Assignment &GetAssignment() const {
            return this->assignment;
        }

        void Interrupt() {
            this->interrupt_requested = true;
        }

        SolverStatus Status() const {
            return this->current_status.load();
        }

        template <typename I>
        SolverStatus Solve(I assumptions_iter, I assumptions_end) {
            if (!this->fresh_solver) {
                static_cast<C *>(this)->ResetState();
            }
            this->fresh_solver = false;
            this->interrupt_requested = false;
            this->current_status = SolverStatus::Solving;

            for (; assumptions_iter != assumptions_end; std::advance(assumptions_iter, 1)) {
                auto literal = *assumptions_iter;
                auto [variable, var_assignment] = literal.Assignment();
                this->pending_assignments.emplace_back(variable, var_assignment, true);
            }

            this->current_status = static_cast<C *>(this)->SolveImpl();
#ifdef SAT_SOLVER_DEBUG_VALIDATIONS_ENABLE
            if (this->current_status == SolverStatus::Satisfied) {
                for (auto [variable, variable_assignment, is_assumption] : this->pending_assignments) {
                    assert(!is_assumption || this->GetAssignment()[variable] != FlipVariableAssignment(variable_assignment));
                }
            }
#endif
            return this->current_status;
        }

        SolverStatus Solve() {
            if (!this->fresh_solver) {
                static_cast<C *>(this)->ResetState();
            }
            this->fresh_solver = false;
            this->interrupt_requested = false;
            this->current_status = SolverStatus::Solving;
            this->current_status = static_cast<C *>(this)->SolveImpl();
            return this->current_status;
        }

        template <typename T>
        friend class ModifiableSolverBase;

     protected:
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

        BaseSolver(const Formula &formula)
            : formula{formula},
            variable_index(formula.NumOfVariables(), VariableIndexEntry{}),
            assignment{formula.NumOfVariables()}, trail{formula.NumOfVariables()} {
            
            this->Rebuild();
        }

        void Rebuild() {
            this->assignment.Reset();
            this->watchers.clear();
            std::fill(this->variable_index.begin(), this->variable_index.end(), VariableIndexEntry{});

            for (std::size_t clause_idx = 0; clause_idx < this->formula.NumOfClauses(); clause_idx++) {
                const auto &clause = this->formula[clause_idx];
                this->watchers.emplace_back(clause);
                this->UpdateClause(clause_idx, clause);
            }
        }

        VariableIndexEntry &VariableIndex(Literal::UInt variable) {
            return this->variable_index[variable - 1];
        }

        void UpdateWatchers(Literal::UInt variable) {
            auto &var_index = this->VariableIndex(variable);
            for (auto affected_watcher : var_index.positive_clauses) {
                this->watchers[affected_watcher].Update(this->assignment, variable);
            }
            for (auto affected_watcher : var_index.negative_clauses) {
                this->watchers[affected_watcher].Update(this->assignment, variable);
            }
        }

        void Assign(Literal::UInt variable, VariableAssignment assignment) {
            this->assignment[variable] = assignment;
            this->UpdateWatchers(variable);
            static_cast<C *>(this)->OnVariableAssignment(variable, assignment);
        }

        std::pair<UnitPropagationResult, std::size_t> UnitPropagation() {
            bool all_satisfied = false, propagate = true;
            while (propagate && !all_satisfied) {
                propagate = false;
                all_satisfied = true;
                for (std::size_t i = 0; !propagate && i < this->watchers.size(); i++) {
                    auto &watcher = this->watchers[i];

                    auto watcher_status = watcher.Status();
                    auto watcher_unit = watcher.WatchedLiterals().first;

                    all_satisfied = all_satisfied && watcher_status == ClauseStatus::Satisfied;
                    if (watcher_status == ClauseStatus::Unit) {
                        auto [variable, assignment] = this->formula[i][watcher_unit].Assignment();

                        this->trail.Propagation(variable, assignment, i);
                        this->Assign(variable, assignment);
                        propagate = true;
                    } else if (watcher_status == ClauseStatus::Unsatisfied) {
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

        void UpdateClause(std::size_t clause_idx, const ClauseView &clause) {
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

        void AttachClause(std::size_t clause_index, const ClauseView &clause) {
            this->ResetStatus();
            this->assignment.SetNumOfVariables(this->formula.NumOfVariables());
            this->trail.SetNumOfVariables(this->formula.NumOfVariables());
            if (this->formula.NumOfVariables() > this->variable_index.size()) {
                this->variable_index.insert(this->variable_index.end(), this->formula.NumOfVariables() - this->variable_index.size(), VariableIndexEntry{});
            }

            auto watcher_iter = this->watchers.emplace(this->watchers.begin() + clause_index, clause);
            this->UpdateClause(clause_index, clause);
            watcher_iter->Rescan(this->assignment);
        }

        void DetachClause(std::size_t index, const ClauseView &) {
            this->ResetStatus();
            this->assignment.SetNumOfVariables(this->formula.NumOfVariables());
            this->trail.SetNumOfVariables(this->formula.NumOfVariables());
            if (this->formula.NumOfVariables() < this->variable_index.size()) {
                this->variable_index.erase(this->variable_index.begin() + this->formula.NumOfVariables(), this->variable_index.end());
            }

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

        void ScanPureLiterals() {
            for (Literal::UInt variable = 1; variable <= this->formula.NumOfVariables(); variable++) {
                auto polarity = this->VariableIndex(variable).polarity;
                if (this->assignment[variable] != VariableAssignment::Unassigned) {
                    continue;
                }

                switch (polarity) {
                    case LiteralPolarity::PurePositive:
                    case LiteralPolarity::None:
                        this->pending_assignments.emplace_back(variable, VariableAssignment::True, false);
                        break;

                    case LiteralPolarity::PureNegative:
                        this->pending_assignments.emplace_back(variable, VariableAssignment::False, false);
                        break;

                    case LiteralPolarity::Mixed:
                        // Intentionally left blank
                        break;
                }
            }
        }

        void OnVariableAssignment(Literal::UInt, VariableAssignment) {}

        void ResetState() {
            this->pending_assignments.clear();
            this->assignment.Reset();
            this->trail.Reset();
            for (auto &watcher : this->watchers) {
                watcher.Rescan(this->assignment);
            }
        }

        bool PerformPendingAssignment(Literal::UInt variable, VariableAssignment variable_assignment, bool is_assumption) {
            auto current_assignment = this->assignment[variable];
            if (is_assumption) {
                if (current_assignment == VariableAssignment::Unassigned) {
                    this->Assign(variable, variable_assignment);
                    this->trail.Assumption(variable, variable_assignment);
                } else if (current_assignment == variable_assignment) {
                    this->trail.Assumption(variable, variable_assignment);
                } else {
                    return false;
                }
            } else if (current_assignment == VariableAssignment::Unassigned) {
                this->Assign(variable, variable_assignment);
                this->trail.Decision(variable, variable_assignment);
            }
            return true;
        }

        template <typename I>
        bool VerifyPendingAssignments(I iter, I end, Literal &conflict) {
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

        const Formula &formula;
        std::vector<VariableIndexEntry> variable_index{};
        std::vector<Watcher> watchers{};
        Assignment assignment;
        DecisionTrail trail;
        std::vector<std::tuple<Literal::UInt, VariableAssignment, bool>> pending_assignments{};
        std::atomic_bool interrupt_requested{false};

        static constexpr std::size_t ClauseUndef = ~static_cast<std::size_t>(0);

     private:
        void ResetStatus() {
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

        bool fresh_solver{true};
        std::atomic<SolverStatus> current_status{SolverStatus::Unknown};
    };

    template <typename C>
    class ModifiableSolverBase {
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
