SUBDIRS =engine

# See https://developer.gnome.org/gtk3/stable/gtk-migrating-2-to-3.html
AM_CPPFLAGS = \
        -I$(top_srcdir)                                              \
        -I$(srcdir)                                                  \
        -g                                                           \
        -DDATA_DIR=\""${datadir}/gmchess"\"                          \
        -DGMCHESS_LOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"    \
        -DGTK_DISABLE_SINGLE_INCLUDES                                \
        -DGDK_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED            \
        -DGSEAL_ENABLE                                               \
        @PACKAGE_CFLAGS@

bin_PROGRAMS=gmchess

gmchess_SOURCES= \
	main.cc \
	Board.cc \
	Engine.cc \
	Pgnfile.cc \
	BookView.cc \
	Sound.cc   \
	robot.cc   \
	ec_throw.cpp \
	ConfWindow.cc \
	variablesmap.cc \
	MainWindow.cc


noinst_HEADERS = \
		 Board.h \
		 gmchess.h \
		 Engine.h \
		 define.h \
		 Pgnfile.h \
		 BookView.h \
		 Sound.h    \
		 robot.h 	\
		 ec_throw.h 	\
		 ConfWindow.h  \
		 variablesmap.h \
		 MainWindow.h

gmchess_LDFLAGS = 

gmchess_LDADD = @LIBS@ @PACKAGE_LIBS@
