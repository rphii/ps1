
headers = [
  ]

src = [
  'main.c',
  ]

cc = meson.get_compiler('c')
dep_rphiic = dependency('rphiic', required: true)

whvn_cli = executable('ps1',
  src,
  dependencies: [dep_rphiic],
  install: true,
)
