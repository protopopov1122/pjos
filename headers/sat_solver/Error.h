#ifndef SAT_SOLVER_ERROR_H_
#define SAT_SOLVER_ERROR_H_

#include "sat_solver/Base.h"
#include <exception>
#include <string>

namespace sat_solver {

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
