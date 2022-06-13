/*
    SPDX-License-Identifier: MIT
    SPDX-FileCopyrightText: 2022 Protopopovs Jevgenijs
*/

#include "pjos/Watcher.h"
#include "pjos/Error.h"
#include <algorithm>
#include <cassert>

namespace pjos {

    Watcher::Watcher(const ClauseView &clause)
        : clause{clause}, status{ClauseStatus::Undecided}, watched_literals{-1, -1} {
        if (this->clause.Length() > 0) {
            this->watched_literals.first = 0;
            if (this->clause.Length() > 1) {
                this->watched_literals.second = 1;
            } else {
                this->status = ClauseStatus::Unit;
            }
        } else {
            this->status = ClauseStatus::Unsatisfied;
        }
    }

    void Watcher::Update(const Assignment &assn, Literal::UInt assigned_variable, VariableAssignment var_assignment, bool satisfies_clause) { // Incremental rescan of clause
#ifdef PJOS_HOTPATH_PARAM_CHECKS
        if (assn.NumOfVariables() < this->clause.NumOfVariables()) {
            throw SatError{"Provided assignment does not cover all clause literals"};
        }
#endif

        if (satisfies_clause) {
            Literal assigned_literal{assigned_variable, var_assignment};
            if (this->status != ClauseStatus::Satisfied &&
                (this->watched_literals.first == -1 || assigned_literal != this->clause[this->watched_literals.first]) &&
                (this->watched_literals.second == -1 || assigned_literal != this->clause[this->watched_literals.second])) { // Watcher is being updated due to new variable assignment
                                                                                                                                        // which satisfied the clause
                auto literal_iter = this->clause.FindLiteral(assigned_literal); // Look for a literal in a clause that could be satisfied by the new assignment
                assert(literal_iter != this->clause.end());
                auto literal_index = std::distance(this->clause.begin(), literal_iter);
                if (!this->IsSatisfied(assn, this->watched_literals.first)) { 
                    this->watched_literals.second = this->watched_literals.first;
                    this->watched_literals.first = literal_index;
                } else if (!this->IsSatisfied(assn, this->watched_literals.second)) {
                    this->watched_literals.second = literal_index;
                }
            }

            this->status = ClauseStatus::Satisfied;
        } else {
            if (this->watched_literals.first != -1 && assigned_variable != this->clause[this->watched_literals.first].Variable() &&
                this->watched_literals.second != -1 && assigned_variable != this->clause[this->watched_literals.second].Variable()) {
                return;
            }
            
            if (this->IsUnsatisfied(assn, this->watched_literals.first)) { // If the first watched literal is false, find an unassigned one
                this->watched_literals.first = this->FindUnassigned(assn, -1);
            }

            if (this->watched_literals.second == this->watched_literals.first ||
                this->IsUnsatisfied(assn, this->watched_literals.second)) { // If the second watched literal is false, or equal the first, find an unassinged one
                this->watched_literals.second = this->FindUnassigned(assn, this->watched_literals.first);
            }

            if (this->status == ClauseStatus::Satisfied &&
                (this->IsSatisfied(assn, this->watched_literals.first) ||
                this->IsSatisfied(assn, this->watched_literals.second))) { // One of watched literals satisfies the clause under current assignment
                this->status = ClauseStatus::Satisfied;
            } else if (this->watched_literals.second != -1) { // There are two unassigned literals
                this->status = ClauseStatus::Undecided;
            } else if (this->watched_literals.first != -1) { // There is one unassigned literal
                this->status = ClauseStatus::Unit;
            } else { // There are no unassigned literals
                this->status = ClauseStatus::Unsatisfied;
            }
        }

#ifdef PJOS_DEBUG_VALIDATIONS_ENABLE
        Watcher clone{*this};
        clone.Rescan(assn);
        if (this->status != clone.status) {
            std::terminate();
        }
#endif
    }

    void Watcher::Rescan(const Assignment &assn) { // Full rescan of the clause
        this->watched_literals = std::make_pair(-1, -1);
        for (std::int64_t i = 0; i < static_cast<std::int64_t>(this->clause.Length()); i++) {
            auto literal = this->clause[i];
            auto var_assignment = assn[literal.Variable()];
            if (literal.Eval(var_assignment)) {
                this->watched_literals.second = this->watched_literals.first;
                this->watched_literals.first = i;
            } else if (var_assignment == VariableAssignment::Unassigned) {
                if (this->watched_literals.first == -1) {
                    this->watched_literals.first = i;
                } else if (this->watched_literals.second == -1) {
                    this->watched_literals.second = i;
                }
            }
        }

        this->UpdateStatus(assn);
    }

    std::int64_t Watcher::FindUnassigned(const Assignment &assn, std::int64_t other) {
        for (std::int64_t i = 0; i < static_cast<std::int64_t>(this->clause.Length()); i++) {
            auto var_assignment = assn[this->clause[i].Variable()];
            if (var_assignment == VariableAssignment::Unassigned && i != other) {
                return i;
            }
        }
        return -1;
    }

    bool Watcher::IsSatisfied(const Assignment &assn, std::int64_t index) {
        if (index == -1) {
            return false;
        }
        auto literal = this->clause[index];
        auto var_assignment = assn[literal.Variable()];
        return literal.Eval(var_assignment);
    }

    bool Watcher::IsUnsatisfied(const Assignment &assn, std::int64_t index) {
        if (index == -1) {
            return true;
        }
        auto literal = this->clause[index];
        auto var_assignment = assn[literal.Variable()];
        return var_assignment != VariableAssignment::Unassigned && !literal.Eval(var_assignment);
    }

    void Watcher::UpdateStatus(const Assignment &assn) {
        if (this->IsSatisfied(assn, this->watched_literals.first) ||
            this->IsSatisfied(assn, this->watched_literals.second)) { // One of watched literals satisfies the clause under current assignment
            this->status = ClauseStatus::Satisfied;
        } else if (this->watched_literals.second != -1) { // There are two unassigned literals
            this->status = ClauseStatus::Undecided;
        } else if (this->watched_literals.first != -1) { // There is one unassigned literal
            this->status = ClauseStatus::Unit;
        } else { // There are no unassigned literals
            this->status = ClauseStatus::Unsatisfied;
        }
    }
}
