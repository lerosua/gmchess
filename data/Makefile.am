SUBDIRS=wood sound
gmchess_ui = \
	     gmchess.glade \
	     p1.png \
	     p2.png \
	     book.dat \
	     gmchess.png


desktopdir       = $(datadir)/applications
desktop_in_files = gmchess.desktop.in
desktop_DATA     = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@


pixmapsdir = $(datadir)/pixmaps
pixmaps_DATA = gmchess.png

gmchessuidir= $(pkgdatadir)
gmchessui_DATA= $(gmchess_ui)

EXTRA_DIST = \
	     $(gmchess_ui) \
	     $(desktop_in_files)
