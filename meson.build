project('TotalVB', ['c'],
  version : '0.0.1',
  license : 'MIT',
  default_options : [
    'warning_level=3',
    # 'optimization=3', # doesn't seem to do anything
    # 'strip=true',
    'c_std=c11',
  ]
)

dependencies = [
]

source = files([
  # core
  'src/core/vb.c',
  'src/core/v810.c',
  'src/core/vip.c',
  'src/core/vsu.c',
  'src/core/timer.c',
  'src/core/mem.c',


  # used for testing
  'src/main.c',
])

includes = include_directories([
  'src',
])

c_flags = [
  '-DVB_DEBUG',
  '-O2',
  '-std=c11'
  # '--param', 'max-inline-insns-single=1000',
  # '--param', 'large-function-growth=100',
]

c_warnings = [
  '-Wall',
  '-Wextra',
  # '-Werror',
  '-Wfatal-errors',
  # '-Wpedantic',
  '-Wshadow',
  '-Wdouble-promotion',
  '-Wformat=2',
  '-Wformat-overflow=2',
  '-Wformat-truncation=1',
  '-Wundef',
  '-Wmissing-include-dirs',
  '-Wstrict-aliasing=2',
  '-Walloca',
  '-Wduplicated-branches',
  '-Wduplicated-cond',
  '-Wwrite-strings',
  '-Wdate-time',
  '-Wsizeof-pointer-memaccess',
  '-Wlogical-op',
  '-Wpacked',
  '-Wredundant-decls',
  '-Wnested-externs',
  '-Winline',
  '-Wcast-qual',
  '-Wcast-align',
  '-Wpointer-arith',
  '-Winit-self',
  '-Wswitch-enum',
  '-Wstrict-prototypes',
  #'-Wmissing-prototypes',
  '-Wredundant-decls',
  '-Wfloat-equal',
  '-Wvla',
  #'-Wc++-compat',
  #'-Wconversion',

  '-fsanitize=address',
  '-fsanitize=leak',
  '-fsanitize=undefined',
  '-fsanitize=pointer-compare',
  '-fsanitize=pointer-subtract',
  '-fstack-protector-all',
]

linkflags = [
  '-fsanitize=address',
  '-fsanitize=leak',
  '-fsanitize=undefined',
  '-fsanitize=pointer-compare',
  '-fsanitize=pointer-subtract',
  '-fstack-protector-all',
]

exe = executable(
  'TotalVB',
  [ source ],
  install: false,
  c_args: [ c_warnings, c_flags ],
  link_args: [ linkflags ],
)

# scripts
run_target('cppcheck',
  command : 'scripts/cppcheck.sh')
