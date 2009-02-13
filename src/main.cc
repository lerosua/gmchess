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
#include <gtkmm/window.h>
#include <gtkmm/main.h>


#ifdef ENABLE_NLS
#  include <libintl.h>
#endif
#include "MainWindow.h"

int main (int argc, char *argv[])
{
	Gtk::Main kit(argc, argv);
	Gtk::Window win;
	MainWindow draw_area;
	win.add(draw_area);
	kit.run(win);
	return 0;
}

