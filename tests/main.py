#!/usr/bin/env python3
import sys
import traceback
import random
from io import StringIO
from sat_solver import SatSolver, SatSolverError
from pysat.formula import CNF
from pysat.solvers import Solver

def generate_formula(num_of_vars, num_of_clauses, max_clause_width):
    formula = CNF()
    assignments = [-var for var in range(1, num_of_vars + 1)]
    assignments.extend(var for var in range(1, num_of_vars + 1))
    for _ in range(num_of_clauses):
        clause_width = random.randint(1, max_clause_width)
        formula.append(random.sample(assignments, clause_width))
    return formula

def solver_test(solver_executable, formula, time_budget):
    solver = Solver(name='cd')
    solver.append_formula(formula)
    
    sat_solver = SatSolver(solver_executable)
    
    dimacs_io = StringIO()
    formula.to_fp(dimacs_io)
    dimacs = dimacs_io.getvalue()
    
    solution = sat_solver.solve(dimacs, time_budget)
    if solution is not None:
        for assn in solution:
            solver.add_clause([assn])
        if not solver.solve():
            raise SatSolverError(f'Model verification failed:\ninput:\n{dimacs}\noutput:\n{solution}')
        return True
    else:
        if solver.solve():
            raise SatSolverError(f'Model verification failed:\ninput:\n{dimacs}\noutput:\n{solution}')
        return False

if __name__ == '__main__':
    try:
        solver_executable = sys.argv[1]
        num_of_variables = int(sys.argv[2])
        num_of_clauses = int(sys.argv[3])
        max_clause_width = int(sys.argv[4])
        num_of_rounds = int(sys.argv[5])
        time_budget = int(sys.argv[6])
        sys.stdout.write(f'Random test(Variables: {num_of_variables}; Clauses: {num_of_clauses}; ' +
                           f'Max. clause width: {max_clause_width}; {num_of_rounds} rounds; Time budget: {time_budget}): ')
        sys.stdout.flush()
        for _ in range(num_of_rounds):
            res = solver_test(sys.argv[1], generate_formula(num_of_variables, num_of_clauses, max_clause_width), time_budget)
            sys.stdout.write('+' if res else '-')
            sys.stdout.flush()
        print()
    except:
        traceback.print_exc()
        exit(-1)
