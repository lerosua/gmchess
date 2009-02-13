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

#include "MainWindow.h"
#include <vector>
const int border_width = 32;
#define	PLACE_LEFT 0x01
#define PLACE_RIGHT 0x02
#define PLACE_ALL PLACE_LEFT | PLACE_RIGHT

MainWindow::MainWindow() 
{
	//this->set_title("Chess");
	//Glib::RefPtr<Gdk::Pixbuf> bg_image = Gdk::Pixbuf::create_from_file("wood.png");
	this->set_size_request(521,577);
	bg_image = Gdk::Pixbuf::create_from_file("wood.png");
	rp_image = Gdk::Pixbuf::create_from_file("rp.png");
	this->set_events(Gdk::BUTTON_PRESS_MASK);
	this->show_all();

}

MainWindow::~MainWindow()
{
}

void MainWindow::get_grid_size(int& width, int& height)
{
	width = (get_width() - border_width * 2) / 8;
	height = (get_height() - border_width * 2) / 9;
}

Gdk::Point MainWindow::get_coordinate(int pos_x, int pos_y)
{
	int grid_width;
	int grid_height;
	get_grid_size(grid_width, grid_height);
	pos_x = pos_x * grid_width + border_width;
	pos_y = pos_y * grid_height + border_width;

	return Gdk::Point(pos_x, pos_y);
}

bool MainWindow::on_expose_event(GdkEventExpose* ev)
{
	draw_bg();
	draw_chessman();
	return true;
}

/**处理点击事件*/
bool MainWindow::on_button_press_event(GdkEventButton* ev)
{
	if(ev->type == GDK_BUTTON_PRESS&& ev->button == 1)
	{
		printf("button press\n");
	Glib::RefPtr<Gdk::Pixbuf> image = Gdk::Pixbuf::create_from_file("oos.png");
	//格式化选中的位置
	int x = (int((ev->x-5)/57))*57+5;
	int y = (int ((ev->y-5)/57))*57+5;
	image->render_to_drawable(get_window(), get_style()->get_black_gc(),
			0, 0, x, y, image->get_width(), image->get_height(), 
			Gdk::RGB_DITHER_NONE, 0, 0);
	}


}

void MainWindow::draw_bg()
{
	//bg_image->render_to_drawable(get_window(), get_style()->get_black_gc(),
			//0, 0, 0, 0, bg_image->get_width(), bg_image->get_height(), 
			//Gdk::RGB_DITHER_NONE, 0, 0);
	
	
	Gdk::Point p1= get_coordinate(0, 0);
	Gdk::Point p2= get_coordinate(8, 9);

	int width = p2.get_x() - p1.get_x();
	int height = p2.get_y() - p1.get_y();

	Glib::RefPtr<Gdk::GC> gc = this->get_style()->get_black_gc();
	gc->set_line_attributes(4, Gdk::LINE_SOLID, Gdk::CAP_NOT_LAST, Gdk::JOIN_ROUND);
	get_window()->draw_rectangle(gc, false, 
			p1.get_x() - 8, p1.get_y() - 8,
			width + 8 * 2, height + 8 * 2);

	gc->set_line_attributes(2, Gdk::LINE_SOLID, Gdk::CAP_NOT_LAST, Gdk::JOIN_ROUND);
	this->get_window()->draw_rectangle(gc, false, 
			p1.get_x(), p1.get_y(),
			width, height);

	gc->set_line_attributes(1, Gdk::LINE_SOLID, Gdk::CAP_NOT_LAST, Gdk::JOIN_ROUND);
	int grid_width;
	int grid_height;
	get_grid_size(grid_width, grid_height);

	for (int i = 0; i < 9; i++) {
		p1 = get_coordinate(0, i);
		p2 = get_coordinate(8, i);
		get_window()->draw_line(gc ,
				p1.get_x(),
				p1.get_y(),
				p2.get_x(),
				p2.get_y());
	}

	for (int i = 0; i < 8; i++) {
		p1 = get_coordinate(i, 0);
		p2 = get_coordinate(i, 4);
		get_window()->draw_line(gc ,
				p1.get_x(),
				p1.get_y(),
				p2.get_x(),
				p2.get_y());
	}

	for (int i = 0; i < 8; i++) {
		p1 = get_coordinate(i, 5);
		p2 = get_coordinate(i, 9);
		get_window()->draw_line(gc ,
				p1.get_x(),
				p1.get_y(),
				p2.get_x(),
				p2.get_y());
	}

	gc->set_line_attributes(2, Gdk::LINE_SOLID, Gdk::CAP_NOT_LAST, Gdk::JOIN_ROUND );

	draw_localize(gc, 0, 3, PLACE_LEFT);
	draw_localize(gc, 8, 3, PLACE_RIGHT);

	for (int i = 0; i < 3; i++) {
		draw_localize(gc, i * 2 + 2, 3, PLACE_ALL);
	}

	draw_localize(gc, 1, 2, PLACE_ALL);
	draw_localize(gc, 7, 2, PLACE_ALL);

	draw_localize(gc, 0, 6, PLACE_LEFT);
	draw_localize(gc, 8, 6, PLACE_RIGHT);

	for (int i = 0; i < 3; i++) {
		draw_localize(gc,  i * 2 + 2, 6, PLACE_ALL);
	}

	draw_localize(gc, 1, 7, PLACE_ALL);
	draw_localize(gc, 7, 7, PLACE_ALL);

	gc->set_line_attributes(1, Gdk::LINE_SOLID, Gdk::CAP_NOT_LAST, Gdk::JOIN_BEVEL );
	draw_palace(gc, 4, 1); 
	draw_palace(gc, 4, 8); 
}

void MainWindow::draw_localize(Glib::RefPtr<Gdk::GC>& gc, int x, int y, int place)
{
	int width;
	int height;
	get_grid_size(width, height);
	width /= 5;
	height /= 5;

	Gdk::Point p = get_coordinate(x, y);

	if (place & PLACE_LEFT) {
		std::vector<Gdk::Point> poss;
		poss.push_back(Gdk::Point(p.get_x() + 5, p.get_y() - height - 4));
		poss.push_back(Gdk::Point(p.get_x() + 5, p.get_y() - 4));
		poss.push_back(Gdk::Point(p.get_x() + 5 + width, p.get_y() - 4));
		this->get_window()->draw_lines(gc, poss);

		poss.clear();
		poss.push_back(Gdk::Point(p.get_x() + 5 + width, p.get_y() + 5));
		poss.push_back(Gdk::Point(p.get_x() + 5, p.get_y() + 5));
		poss.push_back(Gdk::Point(p.get_x() + 5 , p.get_y() + 5 + height));
		this->get_window()->draw_lines(gc, poss);
	}

	if (place & PLACE_RIGHT) {
		std::vector<Gdk::Point> poss;
		poss.push_back(Gdk::Point(p.get_x() - 4 - width, p.get_y() - 4));
		poss.push_back(Gdk::Point(p.get_x() - 4 , p.get_y() - 4));
		poss.push_back(Gdk::Point(p.get_x() - 4 , p.get_y() - 4 - height));
		this->get_window()->draw_lines(gc, poss);
		poss.clear();
		poss.push_back(Gdk::Point(p.get_x() - 4 - width, p.get_y() + 5));
		poss.push_back(Gdk::Point(p.get_x() - 4 , p.get_y() + 5));
		poss.push_back(Gdk::Point(p.get_x() - 4 , p.get_y() + 5 + height));
		this->get_window()->draw_lines(gc, poss);
	}
}

void MainWindow::draw_palace(Glib::RefPtr<Gdk::GC>& gc, int x, int y)
{
	int width;
	int height;
	get_grid_size(width, height);
	Gdk::Point p = get_coordinate(x, y);

	get_window()->draw_line(gc, p.get_x() - width, p.get_y() - height, p.get_x() + width, p.get_y() + height);
	get_window()->draw_line(gc, p.get_x() + width, p.get_y() - height, p.get_x() - width, p.get_y() + height);
}
void MainWindow::draw_chessman()
{
	Glib::RefPtr<Gdk::Pixbuf> image = Gdk::Pixbuf::create_from_file("rp_p.png");
	image->render_to_drawable(get_window(), get_style()->get_black_gc(),
			0, 0, 5+57 * 6, 5+57*2, image->get_width(), image->get_height(), 
			Gdk::RGB_DITHER_NONE, 0, 0);

	rp_image->render_to_drawable(get_window(), get_style()->get_black_gc(),
			0, 0, 5+57 * 6, 5+57*1, image->get_width(), image->get_height(), 
			Gdk::RGB_DITHER_NONE, 0, 0);

	image = Gdk::Pixbuf::create_from_file("rp_e.png");
	image->render_to_drawable(get_window(), get_style()->get_black_gc(),
			0, 0, 5+57 * 7, 5+57 * 1, image->get_width(), image->get_height(), 
			Gdk::RGB_DITHER_NONE, 0, 0);
	image = Gdk::Pixbuf::create_from_file("rk.png");
	image->render_to_drawable(get_window(), get_style()->get_black_gc(),
			0, 0, 5+57 * 7, 5+57 * 0, image->get_width(), image->get_height(), 
			Gdk::RGB_DITHER_NONE, 0, 0);
}
