#!/usr/bin/env python3
import sys
import traceback
import random
from cnfgen import PigeonholePrinciple, RandomKCNF, CountingPrinciple, Shuffle, XorSubstitution
from solver_test import run_solver_test

def alg_php(solver_executable, time_budget, pigeons_min, pigeons_max, holes_min, holes_max):
    pigeons = random.randint(pigeons_min, pigeons_max)
    holes = random.randint(holes_min, holes_max)
    cnf = Shuffle(PigeonholePrinciple(pigeons, holes))
    dimacs = cnf.dimacs()
    return run_solver_test(solver_executable, dimacs, time_budget)

def alg_random(solver_executable, time_budget, k, n, m):
    cnf = XorSubstitution(RandomKCNF(k, n, m), 2)
    dimacs = cnf.dimacs()
    return run_solver_test(solver_executable, dimacs, time_budget)

def alg_count(solver_executable, time_budget, m_min, m_max, p_min, p_max):
    m = random.randint(m_min, m_max)
    p = random.randint(p_min, p_max)
    cnf = Shuffle(CountingPrinciple(m, p))
    dimacs = cnf.dimacs()
    return run_solver_test(solver_executable, dimacs, time_budget)

Algorithms = {
    'php': alg_php,
    'random': alg_random,
    'count': alg_count
}

if __name__ == '__main__':
    try:
        solver_executable = sys.argv[1]
        num_of_rounds = int(sys.argv[2])
        time_budget = int(sys.argv[3])
        algorithm = sys.argv[4]
        parameters = [int(param) for param in sys.argv[5:]]
        algorithm_fn = Algorithms[algorithm]

        sys.stdout.write(f'Generator test({num_of_rounds} rounds; {algorithm} {parameters}; Time budget: {time_budget}): ')
        sys.stdout.flush()
        for _ in range(num_of_rounds):
            res = algorithm_fn(solver_executable, time_budget, *parameters)
            sys.stdout.write('+' if res else '-')
            sys.stdout.flush()
        print()
    except:
        traceback.print_exc()
        exit(-1)
