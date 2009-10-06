/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * main.cc
 * Copyright (C) wind 2009 <xihels@gmail.com>
 * 
 */

//#include <libglademm/xml.h>
#include <iostream>
#include <gtkmm.h>
#include <gtkmm/main.h>
#include <glib/gi18n.h>
#include "MainWindow.h"
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#ifdef ENABLE_NLS
#  include <libintl.h>
#endif

TGMConf GMConf;

int main (int argc, char *argv[])
{

	bindtextdomain (GETTEXT_PACKAGE, GMCHESS_LOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	Gtk::Main kit(argc, argv);
	MainWindow win;
	//Board board;
	//win.add(board);
	kit.run(win);
	return 0;
}

