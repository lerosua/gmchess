bin_PROGRAMS= eleeye_engine convert_pgn

eleeye_engine_CXXFLAGS= $(AM_CPPFLAGS)
convert_pgn_CXXFLAGS=   $(AM_CPPFLAGS)
eleeye_engine_SOURCES= \
		book.cpp \
		eleeye.cpp \
		genmoves.cpp \
		hash.cpp \
		movesort.cpp \
		pipe.cpp \
		position.cpp \
		pregen.cpp \
		search.cpp \
		ucci.cpp
convert_pgn_SOURCES = \
		      convert_pgn.cpp \
		      pregen.cpp \
		      position.cpp \
		      genmoves.cpp  \
		      cchess.cpp \
		      pgnfile.cpp

eleeye_engine_LDFLAGS= -ldl
convert_pgn_LDFLAGS= -ldl
AM_CPPFLAGS =
