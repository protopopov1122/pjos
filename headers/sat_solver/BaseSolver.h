#ifndef SAT_SOLVER_BASE_SOLVER_H_
#define SAT_SOLVER_BASE_SOLVER_H_

#include "sat_solver/Core.h"
#include "sat_solver/Formula.h"
#include "sat_solver/Assignment.h"
#include "sat_solver/Watcher.h"
#include "sat_solver/DecisionTrail.h"
#include <algorithm>
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
            assignment{static_cast<std::size_t>(formula.NumOfVariables())}, trail{static_cast<std::size_t>(formula.NumOfVariables())} {
            
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

        VariableIndexEntry &VariableIndex(Literal::Int variable) {
            return this->variable_index[variable - 1];
        }

        void UpdateWatchers(Literal::Int variable) {
            auto &var_index = this->VariableIndex(variable);
            for (auto affected_watcher : var_index.positive_clauses) {
                this->watchers[affected_watcher].Update(this->assignment, variable);
            }
            for (auto affected_watcher : var_index.negative_clauses) {
                this->watchers[affected_watcher].Update(this->assignment, variable);
            }
        }

        void Assign(Literal::Int variable, VariableAssignment assignment) {
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
            this->assignment.SetNumOfVariables(this->formula.NumOfVariables());
            this->trail.SetNumOfVariables(this->formula.NumOfVariables());
            if (static_cast<std::size_t>(this->formula.NumOfVariables()) > this->variable_index.size()) {
                this->variable_index.insert(this->variable_index.end(), this->formula.NumOfVariables() - this->variable_index.size(), VariableIndexEntry{});
            }

            auto watcher_iter = this->watchers.emplace(this->watchers.begin() + clause_index, clause);
            this->UpdateClause(clause_index, clause);
            watcher_iter->Rescan(this->assignment);
        }

        void AssignPureLiterals() {
            for (Literal::Int variable = 1; variable <= this->formula.NumOfVariables(); variable++) {
                auto polarity = this->VariableIndex(variable).polarity;
                if (this->assignment[variable] != VariableAssignment::Unassigned) {
                    continue;
                }

                switch (polarity) {
                    case LiteralPolarity::PurePositive:
                    case LiteralPolarity::None:
                        this->trail.Decision(variable, VariableAssignment::True);
                        this->Assign(variable, VariableAssignment::True);
                        break;

                    case LiteralPolarity::PureNegative:
                        this->trail.Decision(variable, VariableAssignment::False);
                        this->Assign(variable, VariableAssignment::False);
                        break;

                    case LiteralPolarity::Mixed:
                        // Intentionally left blank
                        break;
                }
            }
        }

        void OnVariableAssignment(Literal::Int, VariableAssignment) {}

        const Formula &formula;
        std::vector<VariableIndexEntry> variable_index{};
        std::vector<Watcher> watchers{};
        Assignment assignment;
        DecisionTrail trail;

        static constexpr std::size_t ClauseUndef = ~static_cast<std::size_t>(0);
    };

    template <typename C>
    class ModifiableSolverBase {
     public:
        const ClauseView &AppendClause(Clause clause) {
            const auto &view = this->owned_formula.AppendClause(std::move(clause));
            static_cast<C *>(this)->AttachClause(this->owned_formula.NumOfClauses() - 1, view);
            return view;
        }

     protected:
        ModifiableSolverBase(Formula formula)
            : owned_formula{std::move(formula)} {}

        Formula owned_formula;
    };
}

#endif
