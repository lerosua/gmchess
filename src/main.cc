/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * main.cc
 * Copyright (C) wind 2009 <xihels@gmail.com>
 * 
 * main.cc is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * main.cc is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <libglademm/xml.h>
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

