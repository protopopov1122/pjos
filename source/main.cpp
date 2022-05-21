#include <cstdlib>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include "sat_solver/Formula.h"
#include "sat_solver/Format.h"
#include "sat_solver/Dimacs.h"
#include "sat_solver/Solver.h"

using namespace sat_solver;

int main(int argc, const char **argv) {
    Formula formula{};

    if (argc > 1) {
        std::ifstream input_file{argv[1]};
        DimacsLoader loader{input_file};
        loader.LoadInto(formula);
    } else {
        DimacsLoader loader{std::cin};
        loader.LoadInto(formula);
    }

    CdclSolver solver(std::move(formula));
    std::vector<Literal> assumptions{};
    if (solver.GetFormula().NumOfVariables() >= 1) {
        assumptions.push_back(1);
    }

    auto status = solver.Solve(assumptions.begin(), assumptions.end());
    if (status == SolverStatus::Unsatisfied && !assumptions.empty()) {
        assumptions[0] = -1;
        status = solver.Solve(assumptions.begin(), assumptions.end());
    }

    std::cout << Format(solver) << std::endl;
    return EXIT_SUCCESS;
}
