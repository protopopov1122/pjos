#include "sat_solver/DecisionTrail.h"

namespace sat_solver {

    DecisionTrail::Entry::Entry(Literal::Int variable, VariableAssignment new_assn, std::int64_t clause_index, std::size_t level)
        : variable{variable}, new_assignment{new_assn}, clause_index{clause_index}, level{level} {}

    void DecisionTrail::Decision(Literal::Int variable, VariableAssignment assn) {
        this->trail.emplace_back(variable, assn, DecisionLabel, ++this->level);
    }

    void DecisionTrail::Propagation(Literal::Int variable, VariableAssignment assn) {
        this->trail.emplace_back(variable, assn, NoClauseLabel, this->level);
    }

    void DecisionTrail::Propagation(Literal::Int variable, VariableAssignment assn, std::size_t clause_index) {
        this->trail.emplace_back(variable, assn, static_cast<std::int64_t>(clause_index), this->level);
    }

    DecisionTrail::Label DecisionTrail::Undo(Literal::Int &variable, VariableAssignment &assignment) {
        if (this->trail.empty()) {
            variable = Literal::Terminator;
            assignment = VariableAssignment::Unassigned;
            return NoClauseLabel;
        }

        auto &entry = this->trail.back();
        variable = entry.variable;
        assignment = entry.new_assignment;
        auto clause_index = entry.clause_index;
        this->trail.pop_back();
        return clause_index;
    }
}
