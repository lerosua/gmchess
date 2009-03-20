SUBDIRS =engine

INCLUDES = 								\
	   -I$(top_srcdir)                                              \
	   -I$(srcdir)                                                  \
	   -g								\
	   -DDATA_DIR=\""${datadir}/gmchess"\" 				\
	   -DGMCHESS_LOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"    \
	   @PACKAGE_CFLAGS@

bin_PROGRAMS=gmchess

gmchess_SOURCES= \
	main.cc	\
	Board.cc \
	Engine.cc \
	Pgnfile.cc \
	MainWindow.cc


noinst_HEADERS = \
		 Board.h \
		 gmchess.h \
		 Engine.h \
		 define.h \
		 Pgnfile.h \
		 MainWindow.h

gmchess_LDFLAGS = 

gmchess_LDADD = @LIBS@ @PACKAGE_LIBS@
