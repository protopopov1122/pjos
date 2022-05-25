## PJOS - Protopopov Jevgenij's Own SAT solver

This repository contains source code of SAT solver developed as a course project for "SAT Solving" course at JKU Linz.
The solver implements CDCL algorithm with EVSIDS (Exponential Variable State Independent Decaying Sum)
heuristic. Additionally, the solver includes simple DPLL algorithm implementation which was used as a baseline for
CDCL testing. SAT Solver supports CNF formulas in DIMACS format, as well as implements IPASIR compatibility layer.
Algorithmically, the inspiration was taken from glucose and minisat projects, albeit this solver is much simpler.
Currently, the solver is in validation and optimization stage.

### Author & License
Author: Protopopovs Jevgenijs

Solver source and test code is licensed under MIT License terms (see `LICENSE`).