#!/usr/bin/env python3
import sys
import traceback
import subprocess
import random
import time
from io import StringIO
from pysat.formula import CNF
from pysat.solvers import Solver

class SolutionError(Exception):
    def __init__(self, formula, status, solution):
        super().__init__(self)
        self.formula = formula
        self.status = status
        self.solution = solution

    def __str__(self) -> str:
        errmsg = StringIO()
        errmsg.write('Formula: ')
        if self.formula:
            self.formula.to_fp(errmsg)
        else:
            errmsg.write('None')
        errmsg.write(f'\nStatus: {self.status}\nOutput: {self.solution}')
        return errmsg.getvalue()

    def __repr__(self) -> str:
        return str(self)

def run_solver(solver_executable, formula, time_budget):
    dimacs = StringIO()
    formula.to_fp(dimacs)

    begin_timestamp = time.time()
    process = subprocess.Popen([solver_executable], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    process.stdin.write(dimacs.getvalue().encode())
    process.stdin.close()
    exit_code = None
    while exit_code is None:
        exit_code = process.poll()
        if time.time() > begin_timestamp + time_budget:
            process.kill()
            raise SolutionError(formula, None, None)
    status = process.stderr.read().decode()
    solution = process.stdout.read().decode()

    if exit_code != 0:
        raise SolutionError(formula, status, solution)

    if status.strip() == 'SATISFIABLE':
        return [int(assn) for assn in solution.split(' ')][:-1]
    elif status.strip() == 'UNSATISFIABLE':
        return None
    else:
        raise SolutionError(formula, status, solution)

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
    
    solution = run_solver(solver_executable, formula, time_budget)
    if solution is not None:
        for assn in solution:
            solver.add_clause([assn])
        if not solver.solve():
            raise SolutionError(formula, 'SAT', solution)
        return True
    else:
        if solver.solve():
            raise SolutionError(formula, 'UNSAT', None)
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
    except SolutionError as ex:
        print(str(ex))
        traceback.print_exc()
        exit(-1)
    except:
        traceback.print_exc()
        exit(-1)
