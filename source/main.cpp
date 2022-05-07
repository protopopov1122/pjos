#include <cstdlib>
#include <iostream>
#include <fstream>
#include "sat_solver/Formula.h"
#include "sat_solver/Format.h"
#include "sat_solver/Dimacs.h"
#include "sat_solver/Solver.h"

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

    DpllSolver solver(formula);
    auto status = solver.Solve();
    std::cerr << Format(status) << std::endl;

    if (status == SolverStatus::Satisfied) {
        Formula constrained_formula{formula};
        for (auto [variable, assignment] : solver.GetAssignment()) {
            formula.AppendClause(ClauseBuilder{}.Add(Literal{variable, assignment}).Make());
        }
        std::cout << Format(formula) << std::endl;
    }
    return EXIT_SUCCESS;
}
