/*
    SPDX-License-Identifier: MIT
    SPDX-FileCopyrightText: 2022 Protopopovs Jevgenijs
*/

#include "pjos/Error.h"

namespace pjos {
    
    SatError::SatError(std::string msg)
        : msg{std::move(msg)} {}

    const char *SatError::what() const noexcept {
        return this->msg.c_str();
    }

    const std::string &SatError::Message() const noexcept {
        return this->msg;
    }
}
