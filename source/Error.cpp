#include "sat_solver/Error.h"

namespace sat_solver {

    SatError::SatError(std::string msg)
        : SatError{SatErrorCode::Unknown, std::move(msg)} {}
    
    SatError::SatError(SatErrorCode ec, std::string msg)
        : error_code{ec}, msg{std::move(msg)} {}

    const char *SatError::what() const noexcept {
        return this->msg.c_str();
    }

    const std::string &SatError::Message() const noexcept {
        return this->msg;
    }

    SatErrorCode SatError::ErrorCode() const noexcept {
        return this->error_code;
    }
}
