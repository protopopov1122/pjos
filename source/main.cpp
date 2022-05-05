#include <cstdlib>
#include <iostream>
#include <fstream>
#include "sat_solver/Formula.h"
#include "sat_solver/Format.h"
#include "sat_solver/Dimacs.h"

using namespace sat_solver;

int main(int argc, const char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " cnf.dimacs" << std::endl;
        return EXIT_FAILURE;
    }

    ClauseContainer clauses;
    Formula formula{clauses};
    std::ifstream input_file{argv[1]};
    DimacsLoader loader{input_file};
    loader.LoadInto(formula);
    std::cout << Format(formula) << std::endl;
    return EXIT_SUCCESS;
}
