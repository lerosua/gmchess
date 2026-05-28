subdir('engine')

core_sources = [
	'Engine.cc',
	'Pgnfile.cc',
  ]

gmchess_core = static_library(
  'gmchess_core',
  core_sources,
  include_directories : [
    include_directories('.'),
    include_directories('..'),
    ],
  install: false,
)

gmchess_sources = [
	'main.cc',
	'Board.cc',
	'BookView.cc',
	'Sound.cc',
	'robot.cc',
	'ec_throw.cpp',
	'ConfWindow.cc',
	'MainWindow.cc',
  ]

if gtk_dep.found()
  executable('gmchess',
    gmchess_sources,
    include_directories : [
      include_directories('.'),
      include_directories('..'),
      ],
    dependencies : [gtk_dep, libiconv_dep],
    link_with : gmchess_core,
    install: true,
  )
else
  message('gtk+-3.0 not found; building core and engine targets only')
endif
