/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * MainWindow.h
 * Copyright (C) wind 2009 <xihels@gmail.com>
 * 
 * MainWindow.h is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * MainWindow.h is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _MAIN_WINDOW_H_
#define _MAIN_WINDOW_H_

#include <gtkmm.h>
#include <iostream>
#include <gtkmm/drawingarea.h>

class MainWindow : public Gtk::DrawingArea {
	public:
		MainWindow();
		~MainWindow();
	protected:
		bool on_expose_event(GdkEventExpose* ev);
		bool on_button_press_event(GdkEventButton* ev);
		void DrawBG();
		void DrawChessman();
	private:
		Glib::RefPtr<Gdk::Pixbuf> bg_image;
		Glib::RefPtr<Gdk::Pixbuf> rp_image;
};

#endif // _MAIN_WINDOW_H_

