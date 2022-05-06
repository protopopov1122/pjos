#include "sat_solver/Watcher.h"
#include "sat_solver/Error.h"

namespace sat_solver {

    Watcher::Watcher(const ClauseView &clause)
        : clause{clause}, status{ClauseStatus::Undecided}, watched_literals{-1, -1} {}

    void Watcher::Update(const Assignment &assn) {
        if (static_cast<Literal::Int>(assn.NumOfVariables()) < this->clause.NumOfVariables()) {
            throw SatError{SatErrorCode::InvalidParameter, "Provided assignment does not cover all clause literals"};
        }

        bool clause_satisfied = false;
        std::pair<std::int64_t, std::int64_t> undecided_idx{-1, -1};
        for (std::size_t i = 0; !clause_satisfied && i < this->clause.Length(); i++) {
            auto literal = this->clause[i];
            auto var_assignment = assn.Of(literal.Variable());
            if (var_assignment != VariableAssignment::Unassigned) {
                clause_satisfied = literal.Eval(var_assignment);
            } else if (undecided_idx.first == -1) {
                undecided_idx.first = i;
            } else if (undecided_idx.second == -1) {
                undecided_idx.second = i;
            }
        }

        if (clause_satisfied) {
            this->status = ClauseStatus::Satisfied;
            this->watched_literals = std::make_pair(-1, -1);
        } else if (undecided_idx.second != -1) {
            this->status = ClauseStatus::Undecided;
            this->watched_literals = std::move(undecided_idx);
        } else if (undecided_idx.first != -1) {
            this->status = ClauseStatus::Unit;
            this->watched_literals = std::move(undecided_idx);
        } else {
            this->status = ClauseStatus::Unsatisfied;
            this->watched_literals = std::make_pair(-1, -1);
        }
    }
}
