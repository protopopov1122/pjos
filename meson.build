project('sat_project', 'cpp', default_options : ['cpp_std=c++17', 'warning_level=3'])

if meson.get_compiler('cpp').get_id() == 'gcc' or meson.get_compiler('cpp').get_id() == 'clang'
  add_project_arguments('-pedantic', language : 'cpp')
endif

sat_solver_source = [
  'source/Clause.cpp',
  'source/ClauseContainer.cpp',
  'source/Dimacs.cpp',
  'source/Error.cpp',
  'source/Format.cpp',
  'source/Formula.cpp',
  'source/Literal.cpp',
  'source/main.cpp'
]

sat_solver_headers = include_directories('headers')
sat_solver = executable('sat_solver', sat_solver_source,
  include_directories : [sat_solver_headers])
