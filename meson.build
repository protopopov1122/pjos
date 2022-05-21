project('sat_project', 'cpp', default_options : ['cpp_std=c++17', 'warning_level=3'])

if meson.get_compiler('cpp').get_id() == 'gcc' or meson.get_compiler('cpp').get_id() == 'clang'
  add_project_arguments('-pedantic', language : 'cpp')

  if get_option('buildtype').startswith('debug')
    add_project_arguments('-DSAT_SOLVER_DEBUG_VALIDATIONS_ENABLE', language : 'cpp')
  endif
endif

sat_solver_lib_source = [
  'source/Assignment.cpp',
  'source/CdclSolver.cpp',
  'source/Clause.cpp',
  'source/DecisionTrail.cpp',
  'source/Dimacs.cpp',
  'source/DpllSolver.cpp',
  'source/Error.cpp',
  'source/Format.cpp',
  'source/Formula.cpp',
  'source/Literal.cpp',
  'source/Watcher.cpp',
  'source/ipasir.cpp'
]

sat_solver_source = [
  'source/main.cpp'
]

sat_solver_headers = include_directories('headers', 'ipasir')
sat_solver_defines = ['-DSAT_SOLVER_IPASIR_INTERFACE_ENABLE', '-DSAT_SOLVER_IPASIR_INTERFACE_ABORT_ON_ERROR']

sat_solver_lib = static_library('sat_solver', sat_solver_lib_source,
  cpp_args : sat_solver_defines,
  include_directories : [sat_solver_headers])

sat_solver = executable('sat_solver', sat_solver_source,
  cpp_args : sat_solver_defines,
  include_directories : [sat_solver_headers],
  link_with : [sat_solver_lib])
