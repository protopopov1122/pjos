#include "pjos/Assignment.h"
#include <algorithm>

namespace pjos {

    Assignment::Assignment(std::size_t num_of_variables)
        : assignment{num_of_variables, VariableAssignment::Unassigned} {}

    Assignment::IteratorType Assignment::begin() const {
        return IteratorType{this->assignment.begin(), 1};
    }

    Assignment::IteratorType Assignment::end() const {
        return IteratorType{this->assignment.end(), this->assignment.size() + 1};
    }

    Assignment &Assignment::Reset() {
        std::fill(this->assignment.begin(), this->assignment.end(), VariableAssignment::Unassigned);
        return *this;
    }

    void Assignment::SetNumOfVariables(std::size_t num_of_variables) {
        if (this->assignment.size() > num_of_variables) {
            this->assignment.erase(this->assignment.begin() + num_of_variables, this->assignment.end());
        } else if (this->assignment.size() < num_of_variables) {
            this->assignment.insert(this->assignment.end(), num_of_variables - this->assignment.size(), VariableAssignment::Unassigned);
        }
    }
}
