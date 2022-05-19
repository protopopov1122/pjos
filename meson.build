project('sat_project', 'cpp', default_options : ['cpp_std=c++17', 'warning_level=3'])

if meson.get_compiler('cpp').get_id() == 'gcc' or meson.get_compiler('cpp').get_id() == 'clang'
  add_project_arguments('-pedantic', language : 'cpp')
endif

sat_solver_source = [
  'source/Assignment.cpp',
  'source/BaseSolver.cpp',
  'source/CdclSolver.cpp',
  'source/Clause.cpp',
  'source/ClauseContainer.cpp',
  'source/DecisionTrail.cpp',
  'source/Dimacs.cpp',
  'source/DpllSolver.cpp',
  'source/Error.cpp',
  'source/Format.cpp',
  'source/Formula.cpp',
  'source/Heuristics.cpp',
  'source/Literal.cpp',
  'source/Watcher.cpp',
  'source/main.cpp'
]

sat_solver_headers = include_directories('headers')
sat_solver = executable('sat_solver', sat_solver_source,
  include_directories : [sat_solver_headers])
