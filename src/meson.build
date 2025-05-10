subdir('engine')

gmchess_sources = [
	'main.cc',
	'Board.cc',
	'Engine.cc',
	'Pgnfile.cc',
	'BookView.cc',
	'Sound.cc',
	'robot.cc',
	'ec_throw.cpp',
	'ConfWindow.cc',
	'variablesmap.cc',
	'MainWindow.cc',
  ]

executable('gmchess',
  gmchess_sources,
  include_directories : [
    include_directories('.'),
    include_directories('..'),
    ],
  dependencies : [gtkmm_dep, libiconv_dep],
  install: true,
)
