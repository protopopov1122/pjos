#include "sat_solver/Solver.h"
#include <algorithm>
#include <cassert>
#include "sat_solver/Format.h"
#include <iostream>

namespace sat_solver {

    BaseSolver::BaseSolver(const Formula &formula)
        : formula{formula},
          variable_index(formula.NumOfVariables(), VariableIndexEntry{}),
          assignment{static_cast<std::size_t>(formula.NumOfVariables())}, trail{static_cast<std::size_t>(formula.NumOfVariables())} {
        
        this->Rebuild();
    }

    void BaseSolver::Rebuild() {
        this->assignment.Reset();
        this->watchers.clear();
        std::fill(this->variable_index.begin(), this->variable_index.end(), VariableIndexEntry{});

        for (std::size_t clause_idx = 0; clause_idx < this->formula.NumOfClauses(); clause_idx++) {
            const auto &clause = this->formula[clause_idx];
            this->watchers.emplace_back(clause);
            this->UpdateClause(clause_idx, clause);
        }
    }

    void BaseSolver::UpdateClause(std::size_t clause_idx, const ClauseView &clause) {
        for (auto literal : clause) {
            auto &index_entry = this->variable_index[literal.Variable() - 1];
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

    void BaseSolver::UpdateWatchers(Literal::Int variable) {
        for (auto affected_watcher : this->variable_index[variable - 1].positive_clauses) {
            this->watchers[affected_watcher].Update(this->assignment, variable);
        }
        for (auto affected_watcher : this->variable_index[variable - 1].negative_clauses) {
            this->watchers[affected_watcher].Update(this->assignment, variable);
        }
    }

    void BaseSolver::Assign(Literal::Int variable, VariableAssignment assignment) {
        this->assignment[variable] = assignment;
        this->UpdateWatchers(variable);
    }

    std::pair<BaseSolver::UnitPropagationResult, std::size_t> BaseSolver::UnitPropagation() {
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

    bool BaseSolver::Backjump(std::size_t level) {
        while (this->trail.Level() > level) {
            auto trail_entry = this->trail.Undo();
            if (!trail_entry.has_value()) {
                return false;
            }

            this->Assign(trail_entry->variable, VariableAssignment::Unassigned);
        }
        return true;
    }

    void BaseSolver::AttachClause(std::size_t clause_index, const ClauseView &clause) {
        if (static_cast<std::size_t>(this->formula.NumOfVariables()) > this->variable_index.size()) {
            this->variable_index.insert(this->variable_index.end(), this->formula.NumOfVariables() - this->variable_index.size(), VariableIndexEntry{});
        }

        auto watcher_iter = this->watchers.emplace(this->watchers.begin() + clause_index, clause);
        this->UpdateClause(clause_index, clause);
        watcher_iter->Rescan(this->assignment);
    }

    void BaseSolver::AssignPureLiterals() {
        for (std::size_t i = 0; i < this->variable_index.size(); i++) {
            auto polarity = this->variable_index[i].polarity;
            auto variable = static_cast<Literal::Int>(i) + 1;
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
}