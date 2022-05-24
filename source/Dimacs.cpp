#include "sat_solver/Dimacs.h"
#include "sat_solver/Error.h"
#include <cctype>
#include <iostream>
#include <string>

namespace sat_solver {

    DimacsLoader::DimacsLoader(std::istream &is)
        : input{is} {}

    void DimacsLoader::LoadInto(Formula &formula) {
        FormulaBuilder builder{formula};
        this->ScanInput(builder);
    }

    void DimacsLoader::ScanInput(FormulaBuilder &builder) {
        this->ScanPreamble();
        while (!this->input.eof()) {
            Literal::Int lit;
            this->input >> lit;
            if (!this->input.good()) {
                break;
            }
            if (lit == 0) {
                builder.EndClause();
            } else {
                builder.AppendLiteral(lit);
            }
        }
    }

    std::pair<std::size_t, Literal::UInt> DimacsLoader::ScanPreamble() {
        static std::string prefix{"p cnf "};
        std::string line;
        while (this->input.good() && !this->input.eof()) {
            std::getline(this->input, line);
            if (line.compare(0, prefix.size(), prefix) == 0) {
                char *strptr = nullptr;
                std::size_t num_of_clauses = std::strtoull(line.c_str() + prefix.size(), &strptr, 10);
                Literal::UInt num_of_variables = std::strtoull(strptr, nullptr, 10);
                return std::make_pair(num_of_clauses, num_of_variables);
            } else if (!line.empty() && line.front() != 'c') {
                throw SatError{"Invalid DIMACS file format"};
            }
        }
        throw SatError{"Invalid DIMACS file format"};
    }
}