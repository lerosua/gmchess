/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * Board.h
 * Copyright (C) wind 2009 <xihels@gmail.com>
 * 
 * Board.h is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Board.h is distributed in the hope that it will be useful, but
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

class Board : public Gtk::DrawingArea {
	public:
		Board();
		~Board();
	protected:
		bool on_expose_event(GdkEventExpose* ev);
		bool on_button_press_event(GdkEventButton* ev);

		Gdk::Point get_coordinate(int pos_x, int pos_y);
		Gdk::Point get_position(int pos_x, int pos_y);
		void get_grid_size(int& width, int& height);

		void draw_bg();
		void draw_chessman();
		void draw_chessman(int x, int y, int chessman_type);
		void draw_select_frame(bool selected = true);
		void draw_localize(Glib::RefPtr<Gdk::GC>& gc, int x, int y, int place);
		void draw_palace(Glib::RefPtr<Gdk::GC>& gc, int x, int y);
	private:
		Glib::RefPtr<Gdk::Pixbuf> bg_image;
		Glib::RefPtr<Gdk::Pixbuf> chessmans[18];
		int selected_x;
		int selected_y;
		int selected_chessman;
};

#endif // _MAIN_WINDOW_H_

