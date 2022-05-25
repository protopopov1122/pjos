#include <cstdlib>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <getopt.h>
#include "sat_solver/Formula.h"
#include "sat_solver/Format.h"
#include "sat_solver/Dimacs.h"
#include "sat_solver/Solver.h"
#include "sat_solver/Error.h"

using namespace sat_solver;
using namespace std::string_literals;

static std::chrono::high_resolution_clock hrclock;

static struct option long_cli_options[] = {
    {"assume", required_argument, 0, 'a'},
    {"quiet", no_argument, 0, 'q'},
    {"learnts", no_argument, 0, 'l'},
    {"no-model", no_argument, 0, 'n'},
    {"version", no_argument, 0, 'v'},
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}
};

static const char *parse_options(int argc, char * const * argv,
                                 std::vector<Literal> &assumptions,
                                 bool &quiet,
                                 bool &print_learned,
                                 bool &include_model) {
    for (;;) {
        int option_index = 0;
        int c = getopt_long (argc, argv, "a:qlnvh", long_cli_options, &option_index);
        if (c == -1) {
            break;
        }
        switch (c) {
            case 'a':
                assumptions.push_back(std::strtoll(optarg, nullptr, 10));
                break;

            case 'q':
                quiet = true;
                break;

            case 'l':
                print_learned = true;
                break;

            case 'n':
                include_model = false;
                break;

            case 'v':
                std::cout << CdclSolver::Signature() << std::endl;
                std::exit(EXIT_SUCCESS);
                break;
            
            case 'h':
                std::cout << CdclSolver::Signature() << std::endl;
                std::cout << "Usage: " << argv[0] << " [options]" << " [DIMACS file]" << std::endl;
                std::cout << "If no DIMACS file is specified, stdin will be used. Options:" << std::endl;
                std::cout << "\t-a,--assume L\tAdd literal L to assumptions" << std::endl;
                std::cout << "\t-q,--quiet\tSuppress auxilary information" << std::endl;
                std::cout << "\t-l,--learnts\tPrint learned clauses" << std::endl;
                std::cout << "\t-n,--no-model\tDo not print satisfying assignment" << std::endl;
                std::cout << "\t-v,--version\tPrint version information" << std::endl;
                std::cout << "\t-h,--help\tPrint this help" << std::endl << std::endl;
                std::cout << "Author: Jevgenijs Protopopovs <jprotopopov1122@gmail.com>" << std::endl;
                std::exit(EXIT_SUCCESS);
                break;

            default:
                throw SatError{"Failed to parse command line options"};
        }
    }
    return optind < argc
        ? argv[optind]
        : nullptr;
}

int main(int argc, char * const *argv) {
    std::vector<Literal> assumptions{};
    bool quiet = false;
    bool print_learned = false;
    bool include_model = true;
    auto cnf_file = parse_options(argc, argv, assumptions, quiet, print_learned, include_model);

    if (!quiet) {
        std::cout << Format(CdclSolver::Signature()) << std::endl;
        std::cout << Format("Input: "s) << (cnf_file != nullptr ? cnf_file : "<stdin>") << std::endl;
    }

    Formula formula{};
    if (cnf_file != nullptr) {
        std::ifstream input_file{cnf_file};
        DimacsLoader loader{input_file};
        loader.LoadInto(formula);
    } else {
        DimacsLoader loader{std::cin};
        loader.LoadInto(formula);
    }

    CdclSolver solver(std::move(formula));

    std::size_t learned_clauses = 0;
    solver.InterruptOn([]() {return false;});

    if (print_learned) {
        solver.OnLearnedClause([&learned_clauses](const auto &clause) {
            learned_clauses++;
            std::cout << Format("Learn clause: "s) << Format(clause) << std::endl;
        });
    } else if (!quiet) {
        solver.OnLearnedClause([&learned_clauses](const auto &) {
            learned_clauses++;
        });
    }

    std::vector<Literal> final_conflict;
    auto begin_timestamp = hrclock.now();
    SolverStatus status;
    if (!assumptions.empty()) {
        status = solver.Solve(assumptions.begin(), assumptions.end(), std::back_inserter(final_conflict));
    } else {
        status = solver.Solve();
    }
    auto duration = hrclock.now() - begin_timestamp;

    if (!quiet) {
        std::cout << Format("Solved in "s) << std::chrono::duration_cast<std::chrono::microseconds>(duration).count() << " microsecond(s)" << std::endl;
        if (status == SolverStatus::Unsatisfied && !assumptions.empty()) {
            std::cout << Format("Final conflict: "s);
            for (auto literal : final_conflict) {
                std::cout << Format(literal) << ' ';
            }
            std::cout << std::endl;
        }
        std::cout << Format("Learned "s) << learned_clauses << " clause(s)" << std::endl;
    }
    std::cout << Format(solver, include_model) << std::endl;
    return EXIT_SUCCESS;
}
