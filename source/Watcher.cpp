#include "sat_solver/Watcher.h"
#include "sat_solver/Error.h"
#include <algorithm>

namespace sat_solver {

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

    void Watcher::Update(const Assignment &assn, Literal::Int assigned_variable) { // Incremental rescan of clause
        if (static_cast<Literal::Int>(assn.NumOfVariables()) < this->clause.NumOfVariables()) {
            throw SatError{SatErrorCode::InvalidParameter, "Provided assignment does not cover all clause literals"};
        }

        if (assigned_variable != Literal::Terminator) { // Watcher is being updated due to new variable assignment
            auto var_assignment = assn[assigned_variable];
            if (var_assignment != VariableAssignment::Unassigned) {
                auto literal_iter = this->clause.FindLiteral(Literal{assigned_variable, var_assignment}); // Look for a literal in a clause that could be satisfied by the new assignment
                auto literal_index = literal_iter != this->clause.end()
                    ? std::distance(this->clause.begin(), literal_iter)
                    : -1;
                if (literal_index != -1 && this->watched_literals.first != literal_index) { // Assigned variable satisfied the clause. Watch corresponding literal,
                                                                                            // if it's not already watched
                    this->watched_literals.second = this->watched_literals.first;
                    this->watched_literals.first = literal_index;
                }
            }
        }

        if (this->IsUnsatisfied(assn, this->watched_literals.first)) { // If the first watched literal is false, find an unassigned one
            this->watched_literals.first = this->FindUnassigned(assn, -1);
        }

        if (this->watched_literals.second == this->watched_literals.first ||
            this->IsUnsatisfied(assn, this->watched_literals.second)) { // If the second watched literal is false, or equal the first, find an unassinged one
            this->watched_literals.second = this->FindUnassigned(assn, this->watched_literals.first);
        }

        this->UpdateStatus(assn);
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
