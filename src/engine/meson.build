eleeye_engine_sources= [
  'book.cpp',
  'eleeye.cpp',
  'genmoves.cpp',
  'hash.cpp',
  'movesort.cpp',
  'pipe.cpp',
  'position.cpp',
  'pregen.cpp',
  'search.cpp',
  'ucci.cpp',
  ]

convert_pgn_sources = [
  'convert_pgn.cpp',
  'pregen.cpp',
  'position.cpp',
  'genmoves.cpp',
  'cchess.cpp',
  'pgnfile.cpp',
  ]

libeval_sources = [
  'preeval.cpp',
  'genmoves.cpp',
  'pregen.cpp',
  'position.cpp',
  'evaluate.cpp',
  ]

libeval = shared_library(
  'eval',
  sources : libeval_sources,
  include_directories : [
    include_directories('.'),
    include_directories('..'),
    ],
  install: true,
  install_dir: libdir
  )

executable('eleeye_engine',
  sources: eleeye_engine_sources,
  include_directories : [
    include_directories('.'),
    include_directories('..'),
    ],
  dependencies : libdl_dep,
  install: true,
  )

executable('convert_pgn',
  sources : convert_pgn_sources,
  include_directories : [
    include_directories('.'),
    include_directories('..'),
    ],
  dependencies : [libdl_dep, libiconv_dep],
  install: true,
  )
