#include "sat_solver/Error.h"

namespace sat_solver {
    
    SatError::SatError(std::string msg)
        : msg{std::move(msg)} {}

    const char *SatError::what() const noexcept {
        return this->msg.c_str();
    }

    const std::string &SatError::Message() const noexcept {
        return this->msg;
    }
}
