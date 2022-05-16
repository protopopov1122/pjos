#include <cstdlib>
#include <iostream>
#include <fstream>
#include "sat_solver/Formula.h"
#include "sat_solver/Format.h"
#include "sat_solver/Dimacs.h"
#include "sat_solver/Solver.h"

using namespace sat_solver;

int main(int argc, const char **argv) {
    ClauseContainer clauses;
    Formula formula{clauses};

    if (argc > 1) {
        std::ifstream input_file{argv[1]};
        DimacsLoader loader{input_file};
        loader.LoadInto(formula);
    } else {
        DimacsLoader loader{std::cin};
        loader.LoadInto(formula);
    }

    DpllSolver solver(formula);
    auto status = solver.Solve();
    std::cerr << Format(status) << std::endl;

    if (status == SolverStatus::Satisfied) {
        ClauseBuilder builder{};
        for (auto [variable, assignment] : solver.GetAssignment()) {
            builder.Add(Literal{variable, assignment});
        }
        std::cout << Format(builder.Make()) << std::endl;
    }
    return EXIT_SUCCESS;
}
