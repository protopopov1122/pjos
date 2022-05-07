#include "sat_solver/Watcher.h"
#include "sat_solver/Error.h"
#include <algorithm>
#include <cassert>

namespace sat_solver {

    Watcher::Watcher(const ClauseView &clause)
        : clause{clause}, status{ClauseStatus::Undecided} {

        // Watch the beginning of the clause
        if (clause.Length() >= 2) {
            this->watched_literals = std::make_pair(0, 1);
        } else if (clause.Length() == 1) {
            this->watched_literals = std::make_pair(0, -1);
        } else {
            this->watched_literals = std::make_pair(-1, -1);
        }
    }

    void Watcher::Update(const Assignment &assn, Literal::Int assigned_variable) { // Incremental rescan of clause
        if (static_cast<Literal::Int>(assn.NumOfVariables()) < this->clause.NumOfVariables()) {
            throw SatError{SatErrorCode::InvalidParameter, "Provided assignment does not cover all clause literals"};
        }

        if (assigned_variable != Literal::Terminator) { // Watcher is being updated due to new variable assignment
            auto var_assignment = assn.Of(assigned_variable);
            if (var_assignment != VariableAssignment::Unassigned) {
                auto literal_index = this->clause.FindLiteral(Literal{assigned_variable, var_assignment});
                if (literal_index != -1) { // Assigned variable satisfied the clause. Watch assigned variable
                    if (this->watched_literals.first != literal_index) {
                        this->watched_literals.second = this->watched_literals.first;
                        this->watched_literals.first = literal_index;
                    }
                    this->status = ClauseStatus::Satisfied;
                    return;
                }
            }
        }

        if (this->IsSatisfied(assn, this->watched_literals.first) ||
            this->IsSatisfied(assn, this->watched_literals.second)) { // One of watched literals satisfies the clause under current assignment
            this->status = ClauseStatus::Satisfied;
            return;
        }

        if (this->watched_literals.second != -1) { // There are two watched literals, none of them satisfied the clause
            auto literal = this->clause[this->watched_literals.second];
            auto var_assignment = assn.Of(literal.Variable());
            if (var_assignment != VariableAssignment::Unassigned) { // The second watched literal was assigned, remove it from watchlist
                this->watched_literals.second = -1;
            }
        }

        if (this->watched_literals.first != -1) { // There is at least one watched literal, it does not satisfy the clause
            auto literal = this->clause[this->watched_literals.first];
            auto var_assignment = assn.Of(literal.Variable());
            if (var_assignment != VariableAssignment::Unassigned) { // The first watched literal was assigned, remove it from watchlist
                this->watched_literals.first = this->watched_literals.second;
                this->watched_literals.second = -1;
            }
        }

        if (this->watched_literals.first == -1) { // There are no watched literals, try to find a new one
            this->watched_literals.first = this->FindUnassigned(assn, -1);
        }

        if (this->watched_literals.first != -1) { // There is at least one unassigned literal
            this->watched_literals.second = this->FindUnassigned(assn, this->watched_literals.first); // Try to find the second one
        } else { // No unassigned literals can be found to watch, clause is unsatisfied
            this->status = ClauseStatus::Unsatisfied;
            return;
        }

        if (this->watched_literals.second != -1) { // There are at least two watched literals, clause is undecided
            this->status = ClauseStatus::Undecided;
        } else { // There is only one watched literal, clause is unit
            this->status = ClauseStatus::Unit;
        }
    }

    std::int64_t Watcher::FindUnassigned(const Assignment &assn, std::int64_t other) {
        for (std::int64_t i = 0; i < static_cast<std::int64_t>(this->clause.Length()); i++) {
            auto var_assignment = assn.Of(this->clause[i].Variable());
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
        auto var_assignment = assn.Of(literal.Variable());
        return literal.Eval(var_assignment);
    }
}
