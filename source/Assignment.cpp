#include "sat_solver/Assignment.h"
#include <algorithm>

namespace sat_solver {

    Assignment::Assignment(std::size_t num_of_variables)
        : assignment{num_of_variables, VariableAssignment::Unassigned} {}

    Assignment::IteratorType Assignment::begin() const {
        return IteratorType{this->assignment.begin(), 1};
    }

    Assignment::IteratorType Assignment::end() const {
        return IteratorType{this->assignment.end(), static_cast<Literal::Int>(this->assignment.size() + 1)};
    }

    Assignment &Assignment::Reset() {
        std::fill(this->assignment.begin(), this->assignment.end(), VariableAssignment::Unassigned);
        return *this;
    }
}
