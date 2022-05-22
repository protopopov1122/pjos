import subprocess
from enum import Enum

class SatSolverError(Exception): pass

class SatResult(Enum):
    Satisfied = 'SATISFIABLE'
    Unsatisfied = 'UNSATISFIABLE'

class SatSolver:
    def __init__(self, executable_path: str):
        self._executable_path = executable_path

    def solve(self, dimacs: str, timeout: float = None):
        process = subprocess.Popen([self._executable_path],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            shell=False)

        try:
            output, error = process.communicate(input=dimacs.encode(), timeout=timeout)
        except subprocess.TimeoutExpired as ex:
            process.terminate()
            raise SatSolverError('SAT solving timeout exceeded')
        if process.returncode != 0:
            raise SatSolverError(f'SAT solver exited with {process.returncode}:\nstdin:\n{dimacs}\nstdout:\n{output}\nstderr:{error}')
        
        output_lines = output.decode().splitlines(keepends=False)
        satisfiability_line = [line for line in output_lines if line.startswith('s ')]
        if len(satisfiability_line) != 1:
            raise SatSolverError(f'Invalid output from SAT solver:\nstdin:\n{dimacs}\nstdout:\n{output}\nstderr:{error}')
        satisfiability = SatResult(satisfiability_line[0][2:])

        if satisfiability == SatResult.Satisfied:
            model_line = [line for line in output_lines if line.startswith('v ')]
            if len(model_line) != 1:
                raise SatSolverError(f'Invalid output from SAT solver:\nstdin:\n{dimacs}\nstdout:\n{output}\nstderr:{error}')
            model = [int(literal) for literal in model_line[0][2:].strip().split(' ')][:-2]
            return model
        else:
            return None
        