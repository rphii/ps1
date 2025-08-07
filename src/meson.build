
headers = [
  ]

src = [
  'ps1-cli.c',
  ]

cc = meson.get_compiler('c')
dep_rlc = dependency('rlc', required: true)
dep_rlso = dependency('rlso', required: true)
dep_rlarg = dependency('rlarg', required: true)

whvn_cli = executable('ps1',
  src,
  dependencies: [dep_rlc, dep_rlso, dep_rlarg],
  install: true,
)
