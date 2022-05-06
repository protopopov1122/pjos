#include "sat_solver/DecisionTrail.h"

namespace sat_solver {

    DecisionTrail::Entry::Entry(Literal::Int variable, VariableAssignment new_assn, VariableAssignment old_assn, bool decision, std::size_t level)
        : variable{variable}, new_assignment{new_assn}, prev_assignment{old_assn}, decision{decision}, level{level} {}

    DecisionTrail::DecisionTrail(Assignment &assignment, std::function<void(Literal::Int, VariableAssignment)> assign_fn)
        : assignment{assignment}, assign_fn{std::move(assign_fn)}, level{0} {}

    void DecisionTrail::Decision(Literal::Int variable, VariableAssignment assn) {
        this->trail.emplace_back(variable, assn, this->assignment.Of(variable), true, ++this->level);
        this->assign_fn(variable, assn);
    }

    void DecisionTrail::Propagation(Literal::Int variable, VariableAssignment assn) {
        this->trail.emplace_back(variable, assn, this->assignment.Of(variable), false, this->level);
        this->assign_fn(variable, assn);
    }

    std::pair<Literal::Int, VariableAssignment> DecisionTrail::UndoDecision() {
        bool undo = true;
        std::pair<Literal::Int, VariableAssignment> last_decision{Literal::Terminator, VariableAssignment::Unassigned};
        while (!this->trail.empty() && undo) {
            auto &entry = this->trail.back();
            if (entry.decision) {
                undo = false;
                last_decision = std::make_pair(entry.variable, entry.new_assignment);
                this->level--;
            }

            this->assign_fn(entry.variable, entry.prev_assignment);
            this->trail.pop_back();
        }
        return last_decision;
    }
}
