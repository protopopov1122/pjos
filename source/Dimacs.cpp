#include "sat_solver/Dimacs.h"
#include "sat_solver/Error.h"
#include <cctype>
#include <iostream>
#include <string>

namespace sat_solver {

    DimacsLoader::DimacsLoader(std::istream &is)
        : input{is} {}

    void DimacsLoader::LoadInto(Formula &formula) {
        this->ScanInput(formula);
    }

    void DimacsLoader::ScanInput(Formula &formula) {
        FormulaBuilder builder{formula};
        auto [num_of_clauses, num_of_variables] = this->ScanPreamble();
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

        if (num_of_clauses != formula.NumOfClauses()) {
            std::cerr << "Warning: Number of clauses does not match DIMACS preamble " << num_of_clauses << ' ' << formula.NumOfClauses() << std::endl;
        }
        if (num_of_variables != formula.NumOfVariables()) {
            std::cerr << "Warning: Number of variables does not match DIMACS preamble" << std::endl;
        }
    }

    std::pair<std::size_t, Literal::UInt> DimacsLoader::ScanPreamble() {
        static std::string prefix{"p cnf "};
        std::string line;
        while (this->input.good() && !this->input.eof()) {
            std::getline(this->input, line);
            if (line.compare(0, prefix.size(), prefix) == 0) {
                char *strptr = nullptr;
                Literal::UInt num_of_variables = std::strtoull(line.c_str() + prefix.size(), &strptr, 10);
                std::size_t num_of_clauses = std::strtoull(strptr, nullptr, 10);
                return std::make_pair(num_of_clauses, num_of_variables);
            } else if (!line.empty() && line.front() != 'c') {
                throw SatError{"Invalid DIMACS file format"};
            }
        }
        throw SatError{"Invalid DIMACS file format"};
    }
}