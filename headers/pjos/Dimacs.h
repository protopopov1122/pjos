/*
    SPDX-License-Identifier: MIT
    SPDX-FileCopyrightText: 2022 Protopopovs Jevgenijs
*/

#ifndef PJOS_DIMACS_H_
#define PJOS_DIMACS_H_

#include "pjos/Formula.h"
#include <iosfwd>

// Simple DIMACS format parser

namespace pjos {

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
