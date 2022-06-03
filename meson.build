project('pjos_solver', 'cpp', default_options : ['cpp_std=c++17', 'warning_level=3'])

if meson.get_compiler('cpp').get_id() == 'gcc' or meson.get_compiler('cpp').get_id() == 'clang'
  add_project_arguments('-pedantic', language : 'cpp')

  if get_option('buildtype') == 'debug'
    add_project_arguments('-DPJOS_DEBUG_VALIDATIONS_ENABLE', language : 'cpp')
  endif
endif

pjos_lib_source = [
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

pjos_source = [
  'source/main.cpp'
]

pjos_headers = [include_directories('headers')]
pjos_defines = []

if get_option('ipasir_support')
  pjos_defines += '-DPJOS_IPASIR_INTERFACE_ENABLE'
endif

if get_option('ipasir_abort_on_error')
  pjos_defines += '-DPJOS_IPASIR_INTERFACE_ABORT_ON_ERROR'
endif

if get_option('ipasir_header') != ''
  pjos_defines += '-DPJOS_IPASIR_INTERFACE_HEADER="' + get_option('ipasir_header') + '"'
elif get_option('ipasir_support')
  pjos_headers += [include_directories('ipasir')]
endif

pjos_lib = static_library('pjos', pjos_lib_source,
  cpp_args : pjos_defines,
  include_directories : pjos_headers)

pjos = executable('pjos', pjos_source,
  cpp_args : pjos_defines,
  include_directories : pjos_headers,
  link_with : [pjos_lib])
