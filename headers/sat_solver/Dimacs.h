#ifndef SAT_SOLVER_DIMACS_H_
#define SAT_SOLVER_DIMACS_H_

#include "sat_solver/Formula.h"
#include <iosfwd>

namespace sat_solver {

    class DimacsLoader {
     public:
        DimacsLoader(std::istream &);
        DimacsLoader(const DimacsLoader &) = delete;
        DimacsLoader(DimacsLoader &&) = delete;

        ~DimacsLoader() = default;

        DimacsLoader &operator=(const DimacsLoader &) = delete;
        DimacsLoader &operator=(DimacsLoader &&) = delete;

        void LoadInto(Formula &);

     private:
        void ScanInput(Formula &);
        std::pair<std::size_t, Literal::UInt> ScanPreamble();

        std::istream &input;
    };
}

#endif
