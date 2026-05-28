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
	'variablesmap.cc',
	'MainWindow.cc',
  ]

if gtkmm_dep.found()
  executable('gmchess',
    gmchess_sources,
    include_directories : [
      include_directories('.'),
      include_directories('..'),
      ],
    dependencies : [gtkmm_dep, libiconv_dep],
    link_with : gmchess_core,
    install: true,
  )
else
  message('gtkmm-3.0 not found; building core and engine targets only')
endif
