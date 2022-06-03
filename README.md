## PJOS - Protopopov Jevgenij's Own SAT Solver

This repository contains source code of SAT solver developed as a course project for "SAT Solving" course at JKU Linz.
Algorithmically, inspiration was taken from [minisat](http://minisat.se/) and [glucose](https://www.labri.fr/perso/lsimon/glucose/) projects.
The solver implements the following concepts:

* Conflict-driven clause learning algorithm.
* Watched literals.
* Exponential Variable State Independent Decaying Sum (EVSIDS) heuristic for CDCL decision guidance.
* Loader of formulas in DIMACS format.
* IPASIR compatibility layer.
* Simple DPLL algorithm implementation.

The solver is written in C++17 language and relies on Meson for building. Besides C++ standard library, solver depends on
[IPASIR](https://github.com/biotomas/ipasir) header in order to define compatible interface, however the compatibility level support can be dropped at compile time.

Testing approach is based on fuzzying with random CNF formulas for a few problem classes
(Pigeonhole principle, Counting principle, randomized formulas). In addition, a few manually selected samples,
which proved to be problematic during the development, are used. Testing scripts are written in Python 3
and depend on a number of libraries available in PyPI (see `tests/requirements.txt`). Testing scripts run solver with formulas
in DIMACS format  and then validate solver output using [CaDiCaL](https://github.com/arminbiere/cadical).

The solver offers command line utility, documentation for it can be obtained via running it with `--help`
option.

### Author & License
Author: Protopopovs Jevgenijs

Solver source and test code is licensed under MIT License terms (see `LICENSE`).

Some of CNF files in `tests/samples` directory are generated using [cnfgen](https://massimolauria.net/cnfgen/) utility and contain
copyright header provided by the utility.