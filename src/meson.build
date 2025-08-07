
headers = [
  ]

src = [
  'ps1-cli.c',
  ]

cc = meson.get_compiler('c')

rlarg_dep = dependency('rlarg',
  fallback : ['rlarg', 'rlarg_dep'],
  default_options: ['default_library=static']
)

rlso_dep = dependency('rlso',
  fallback : ['rlso', 'rlso_dep'],
  default_options: ['default_library=static']
)

rlc_dep = dependency('rlc',
  fallback : ['rlc', 'rlc_dep'],
  default_options: ['default_library=static']
)


whvn_cli = executable('ps1',
  src,
  dependencies: [rlc_dep, rlso_dep, rlarg_dep],
  install: true,
)
