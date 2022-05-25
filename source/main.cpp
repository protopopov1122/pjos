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
using namespace std::string_literals;

static std::chrono::high_resolution_clock hrclock;

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

    std::size_t learned_clauses = 0;
    solver.InterruptOn([]() {return false;});
    solver.OnLearnedClause([&learned_clauses](const auto &) {
        learned_clauses++;
    });

    std::vector<Literal> final_conflict;
    auto begin_timestamp = hrclock.now();
    auto status = solver.Solve(assumptions.begin(), assumptions.end(), std::back_inserter(final_conflict));
    if (status == SolverStatus::Unsatisfied && !assumptions.empty()) {
        assumptions[0] = -1;
        status = solver.Solve(assumptions.begin(), assumptions.end());
    }
    auto duration = hrclock.now() - begin_timestamp;

    std::cout << Format(solver.Signature()) << std::endl;
    std::cout << Format("Input cnf: "s) << (argc > 1 ? argv[1] : "<stdin>") << std::endl;
    std::cout << Format("Solved in "s) << std::chrono::duration_cast<std::chrono::microseconds>(duration).count() << " microseconds" << std::endl;
    if (status == SolverStatus::Unsatisfied && !assumptions.empty()) {
        std::cout << Format("Final conflict: "s);
        for (auto literal : final_conflict) {
            std::cout << Format(literal) << ' ';
        }
        std::cout << std::endl;
    }
    std::cout << Format("Learned "s) << learned_clauses << " clause(s)" << std::endl;
    std::cout << Format(solver) << std::endl;
    return EXIT_SUCCESS;
}
