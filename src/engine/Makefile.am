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

noinst_HEADERS = \
		base2.h 	\
		base.h 	\
		book.h 	\
		cchess.h 	\
		CodeConverter.h \
		ecco.h 	\
		hash.h 	\
		movesort.h 	\
		parse.h   	\
		pgnfile.h 	\
		pipe.h   	\
		position.h 	\
		pregen.h	\
		rc4prng.h	\
		search.h	\
		ucci.h		\
		x86asm.h	\
		xqffile.h


eleeye_engine_LDFLAGS= -ldl
convert_pgn_LDFLAGS= -ldl
AM_CPPFLAGS =

EXTRA_DIST = $(noinst_HEADERS)
