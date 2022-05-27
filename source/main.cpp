#include <cstdlib>
#include <iostream>
#include <fstream>
#include <chrono>
#include <getopt.h>
#include "pjos/Formula.h"
#include "pjos/Format.h"
#include "pjos/Dimacs.h"
#include "pjos/DpllSolver.h"
#include "pjos/CdclSolver.h"
#include "pjos/Error.h"

using namespace pjos;
using namespace std::string_literals;

static std::chrono::high_resolution_clock hrclock;

static struct option long_cli_options[] = {
    {"assume", required_argument, 0, 'a'},
    {"quiet", no_argument, 0, 'q'},
    {"learnts", no_argument, 0, 'l'},
    {"no-model", no_argument, 0, 'n'},
    {"use-dpll", no_argument, 0, 'D'},
    {"version", no_argument, 0, 'v'},
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}
};

struct Options {
    std::vector<Literal> assumptions{};
    bool quiet{false};
    bool print_learned{false};
    bool include_model{true};
    bool use_dpll{false};
    const char *cnf_file;
};

static bool parse_options(int argc, char * const * argv, Options &options) {
    for (;;) {
        int option_index = 0;
        int c = getopt_long (argc, argv, "a:qlnDvh", long_cli_options, &option_index);
        if (c == -1) {
            break;
        }
        switch (c) {
            case 'a':
                options.assumptions.push_back(std::strtoll(optarg, nullptr, 10));
                break;

            case 'q':
                options.quiet = true;
                break;

            case 'l':
                options.print_learned = true;
                break;

            case 'n':
                options.include_model = false;
                break;

            case 'D':
                options.use_dpll = true;
                break;

            case 'v':
                std::cout << PJOS_VERSION << std::endl;
                return true;
            
            case 'h':
                std::cout << PJOS_IDENTIFIER << ' ' << PJOS_VERSION << std::endl;
                std::cout << "Usage: " << argv[0] << " [options]" << " [DIMACS file]" << std::endl;
                std::cout << "If no DIMACS file is specified, stdin will be used. Options:" << std::endl;
                std::cout << "\t-a,--assume L\tAdd literal L to assumptions" << std::endl;
                std::cout << "\t-q,--quiet\tSuppress auxilary information" << std::endl;
                std::cout << "\t-l,--learnts\tPrint learned clauses (available only for CDCL solver)" << std::endl;
                std::cout << "\t-n,--no-model\tDo not print satisfying assignment" << std::endl;
                std::cout << "\t-D,--use-dpll\tUse DPLL solver instead of CDCL" << std::endl;
                std::cout << "\t-v,--version\tPrint version information" << std::endl;
                std::cout << "\t-h,--help\tPrint this help" << std::endl << std::endl;
                std::cout << "Author: Jevgenijs Protopopovs <jprotopopov1122@gmail.com>" << std::endl;
                std::cout << "URL: <https://github.com/protopopov1122/pjos>" << std::endl;
                return true;

            default:
                throw SatError{"Failed to parse command line options"};
        }
    }
    options.cnf_file = optind < argc
        ? argv[optind]
        : nullptr;

    if (options.use_dpll && options.print_learned) {
        throw SatError{"DPLL solver has no support for learned clauses"};
    }
    return false;
}

template <typename T>
static void print_greeting(const Options &options) {
    if (!options.quiet) {
        std::cout << Format(T::Signature()) << std::endl;
        std::cout << Format("Input: "s) << (options.cnf_file != nullptr ? options.cnf_file : "<stdin>") << std::endl;
    }
}

static Formula load_formula(const Options &options) {
    Formula formula{};
    if (options.cnf_file != nullptr) {
        std::ifstream input_file{options.cnf_file};
        DimacsLoader loader{input_file};
        loader.LoadInto(formula);
    } else {
        DimacsLoader loader{std::cin};
        loader.LoadInto(formula);
    }
    return formula;
}

static void setup_cdcl_callbacks(const Options &options, CdclSolver &solver, std::size_t &learned_clauses) {
    solver.InterruptOn([]() {return false;});
    if (options.print_learned) {
        solver.OnLearnedClause([&learned_clauses](const auto &clause) {
            learned_clauses++;
            std::cout << Format("Learn clause: "s) << Format(clause) << std::endl;
        });
    } else if (!options.quiet) {
        solver.OnLearnedClause([&learned_clauses](const auto &) {
            learned_clauses++;
        });
    }
}

static void run_cdcl_solver(const Options &options) {
    print_greeting<CdclSolver>(options);

    std::size_t learned_clauses = 0;
    CdclSolver solver(load_formula(options));
    setup_cdcl_callbacks(options, solver, learned_clauses);

    std::vector<Literal> final_conflict;
    auto begin_timestamp = hrclock.now();
    SolverStatus status;
    if (!options.assumptions.empty()) {
        status = solver.Solve(options.assumptions.begin(), options.assumptions.end(), std::back_inserter(final_conflict));
    } else {
        status = solver.Solve();
    }
    auto duration = hrclock.now() - begin_timestamp;

    if (!options.quiet) {
        std::cout << Format("Solved in "s) << std::chrono::duration_cast<std::chrono::microseconds>(duration).count() << " microsecond(s)" << std::endl;
        if (status == SolverStatus::Unsatisfied && !options.assumptions.empty()) {
            std::cout << Format("Final conflict: "s);
            for (auto literal : final_conflict) {
                std::cout << Format(literal) << ' ';
            }
            std::cout << std::endl;
        }
        std::cout << Format("Learned "s) << learned_clauses << " clause(s)" << std::endl;
    }
    std::cout << Format(solver, options.include_model) << std::endl;
}

static void run_dpll_solver(const Options &options) {
    print_greeting<ModifiableDpllSolver>(options);

    ModifiableDpllSolver solver(load_formula(options));
    solver.InterruptOn([]() {return false;});

    auto begin_timestamp = hrclock.now();
    if (!options.assumptions.empty()) {
        solver.Solve(options.assumptions.begin(), options.assumptions.end());
    } else {
        solver.Solve();
    }
    auto duration = hrclock.now() - begin_timestamp;

    if (!options.quiet) {
        std::cout << Format("Solved in "s) << std::chrono::duration_cast<std::chrono::microseconds>(duration).count() << " microsecond(s)" << std::endl;
    }
    std::cout << Format(solver, options.include_model) << std::endl;
}

int main(int argc, char * const *argv) {
    Options options{};
    if (parse_options(argc, argv, options)) {
        return EXIT_SUCCESS;
    }
    
    if (!options.use_dpll) {
        run_cdcl_solver(options);
    } else {
        run_dpll_solver(options);
    }
    return EXIT_SUCCESS;
}
