/*
    SPDX-License-Identifier: MIT
    SPDX-FileCopyrightText: 2022 Protopopovs Jevgenijs
*/

#ifndef PJOS_ERROR_H_
#define PJOS_ERROR_H_

#include "pjos/Base.h"
#include <exception>
#include <string>

// Exception class used in all throw statements pefromed by the solver

namespace pjos {

    class SatError : public std::exception {
     public:
        SatError(std::string);
        SatError(const SatError &) = default;
        SatError(SatError &&) = default;

        ~SatError() = default;

        SatError &operator=(const SatError &) = default;
        SatError &operator=(SatError &&) = default;

        const char *what() const noexcept final;
        const std::string &Message() const noexcept;

     private:
        std::string msg;
    };
}

#endif
