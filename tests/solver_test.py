from pysat.formula import CNF
from pysat.solvers import Solver
from sat_solver import SatSolver, SatSolverError

def run_solver_test(solver_executable, dimacs, time_budget):
    solver = Solver(name='cd')
    solver.append_formula(CNF(from_string=dimacs))
    
    sat_solver = SatSolver(solver_executable)
    
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
