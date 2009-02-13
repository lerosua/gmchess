## Process this file with automake to produce Makefile.in

## Created by Anjuta

gladedir = $(datadir)/chess/glade
glade_DATA = chess.glade

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	$(CHESS_CFLAGS)

AM_CFLAGS =\
	 -Wall\
	 -g

bin_PROGRAMS = chess

chess_SOURCES = \
	main.cc	\
	MainWindow.cc

chess_LDFLAGS = 

chess_LDADD = $(CHESS_LIBS)

EXTRA_DIST = $(glade_DATA)
