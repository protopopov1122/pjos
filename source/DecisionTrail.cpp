#include "sat_solver/DecisionTrail.h"

namespace sat_solver {

    DecisionTrail::Entry::Entry(Literal::Int variable, VariableAssignment new_assn, bool decision, std::size_t level)
        : variable{variable}, new_assignment{new_assn}, decision{decision}, level{level} {}

    void DecisionTrail::Decision(Literal::Int variable, VariableAssignment assn) {
        this->trail.emplace_back(variable, assn, true, ++this->level);
    }

    void DecisionTrail::Propagation(Literal::Int variable, VariableAssignment assn) {
        this->trail.emplace_back(variable, assn, false, this->level);
    }

    bool DecisionTrail::Undo(Literal::Int &variable, VariableAssignment &assignment) {
        if (this->trail.empty()) {
            variable = Literal::Terminator;
            assignment = VariableAssignment::Unassigned;
            return false;
        }

        auto &entry = this->trail.back();
        variable = entry.variable;
        assignment = entry.new_assignment;
        bool decision = entry.decision;
        this->trail.pop_back();
        return !decision;
    }
}
