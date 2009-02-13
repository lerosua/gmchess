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

bool MainWindow::on_expose_event(GdkEventExpose* ev)
{
	DrawBG();
	DrawChessman();
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

void MainWindow::DrawBG()
{
	//bg_image->render_to_drawable(get_window(), get_style()->get_black_gc(),
			//0, 0, 0, 0, bg_image->get_width(), bg_image->get_height(), 
			//Gdk::RGB_DITHER_NONE, 0, 0);
	
	int width = (this->get_width() - border_width * 2) / 8 * 8 + border_width * 2;
	int height = (this->get_height() - border_width * 2) / 9 * 9 + border_width * 2;
	Glib::RefPtr<Gdk::GC> gc = this->get_style()->get_black_gc();
	gc->set_line_attributes(4, Gdk::LINE_SOLID, Gdk::CAP_NOT_LAST, Gdk::JOIN_ROUND);
	this->get_window()->draw_rectangle(gc, false, border_width - 8, border_width - 8, width - border_width * 2 + 8 * 2, height - border_width * 2 + 8 * 2);

	gc->set_line_attributes(2, Gdk::LINE_SOLID, Gdk::CAP_NOT_LAST, Gdk::JOIN_ROUND);
	this->get_window()->draw_rectangle(gc, false, border_width, border_width, width - border_width * 2 , height - border_width * 2);

	gc->set_line_attributes(1, Gdk::LINE_SOLID, Gdk::CAP_NOT_LAST, Gdk::JOIN_ROUND);
	int grid_width = (width - border_width * 2) / 8;
	int grid_height = (height - border_width * 2) / 9;
	for (int i = 0; i < 9; i++) {
		this->get_window()->draw_line(gc, border_width,
			   	grid_height * i + border_width, 
				width - border_width, 
				grid_height * i + border_width);
	}

	for (int i = 0; i < 8; i++) {
		this->get_window()->draw_line(gc, grid_width * i + border_width,
			   	border_width,
				grid_width * i + border_width, 
				grid_height * 4 + border_width);
	}

	for (int i = 0; i < 8; i++) {
		this->get_window()->draw_line(gc, grid_width * i + border_width,
			   	grid_height * 5 + border_width,
				grid_width * i + border_width, 
				grid_height * 9 + border_width);
	}

	gc->set_line_attributes(2, Gdk::LINE_SOLID, Gdk::CAP_NOT_LAST, Gdk::JOIN_ROUND );

	DrawLocalize(gc, border_width, grid_height * 3 + border_width, grid_width, grid_height, PLACE_LEFT);
	DrawLocalize(gc, grid_width * 8 + border_width, grid_height * 3 + border_width, grid_width, grid_height, PLACE_RIGHT);

	for (int i = 0; i < 3; i++) {
		DrawLocalize(gc, grid_width * (i * 2 + 2)  + border_width, grid_height * 3 + border_width, grid_width, grid_height, PLACE_ALL);
	}

		DrawLocalize(gc, grid_width * 1  + border_width, grid_height * 2 + border_width, grid_width, grid_height, PLACE_ALL);
		DrawLocalize(gc, grid_width * 7  + border_width, grid_height * 2 + border_width, grid_width, grid_height, PLACE_ALL);

	DrawLocalize(gc, border_width, grid_height * 6 + border_width, grid_width, grid_height, PLACE_LEFT);
	DrawLocalize(gc, grid_width * 8 + border_width, grid_height * 6 + border_width, grid_width, grid_height, PLACE_RIGHT);

	for (int i = 0; i < 3; i++) {
		DrawLocalize(gc, grid_width * (i * 2 + 2)  + border_width, grid_height * 6 + border_width, grid_width, grid_height, PLACE_ALL);
	}

	DrawLocalize(gc, grid_width * 1  + border_width, grid_height * 7 + border_width, grid_width, grid_height, PLACE_ALL);
	DrawLocalize(gc, grid_width * 7  + border_width, grid_height * 7 + border_width, grid_width, grid_height, PLACE_ALL);

	gc->set_line_attributes(1, Gdk::LINE_SOLID, Gdk::CAP_NOT_LAST, Gdk::JOIN_BEVEL );
	DrawPalace(gc, grid_width * 4 + border_width, border_width, grid_width, grid_height);
}

void MainWindow::DrawLocalize(Glib::RefPtr<Gdk::GC>& gc, int x, int y, int grid_width, int grid_height, int place)
{
	int width = grid_width / 5;
	int height = grid_height / 5;
	if (place & PLACE_LEFT) {
		std::vector<Gdk::Point> pos;
		pos.push_back(Gdk::Point(x + 5, y - height - 4));
		pos.push_back(Gdk::Point(x + 5, y - 4));
		pos.push_back(Gdk::Point(x + 5 + width, y - 4));
		this->get_window()->draw_lines(gc, pos);
		//this->get_window()->draw_line(gc, x + 4, y - height - 4, x + 4, y - 4);
		//this->get_window()->draw_line(gc, x + 4, y - 4, x + 4 + width, y - 4);

		pos.clear();
		pos.push_back(Gdk::Point(x + 5 + width, y + 5));
		pos.push_back(Gdk::Point(x + 5, y + 5));
		pos.push_back(Gdk::Point(x + 5 , y + 5 + height));
		this->get_window()->draw_lines(gc, pos);
		//this->get_window()->draw_line(gc, x + 4, y + 4, x + 4 , y + 4 + height);
		//this->get_window()->draw_line(gc, x + 4, y + 4, x + 4 + width , y + 4);
	}

	if (place & PLACE_RIGHT) {
		std::vector<Gdk::Point> pos;
		pos.push_back(Gdk::Point(x - 4 - width, y - 4));
		pos.push_back(Gdk::Point(x - 4 , y - 4));
		pos.push_back(Gdk::Point(x - 4 , y - 4 - height));
		this->get_window()->draw_lines(gc, pos);
		//this->get_window()->draw_line(gc, x - 4, y - height - 4, x - 4, y - 4);
		//this->get_window()->draw_line(gc, x - 4 - width, y - 4, x - 4 , y - 4);
		pos.clear();
		pos.push_back(Gdk::Point(x - 4 - width, y + 5));
		pos.push_back(Gdk::Point(x - 4 , y + 5));
		pos.push_back(Gdk::Point(x - 4 , y + 5 + height));
		this->get_window()->draw_lines(gc, pos);
		//this->get_window()->draw_line(gc, x - 4, y + 4, x - 4 , y + 4 + height);
		//this->get_window()->draw_line(gc, x - 4 - width, y + 4, x - 4 , y + 4);
	}
}

void MainWindow::DrawPalace(Glib::RefPtr<Gdk::GC>& gc, int x, int y, int grid_width, int grid_height )
{
	this->get_window()->draw_line(gc, x - grid_width, y, x + grid_width, y + grid_height * 2);
	this->get_window()->draw_line(gc, x - grid_width, y + grid_height * 2, x + grid_width, y);

	this->get_window()->draw_line(gc, x - grid_width, y + grid_height * 7, x + grid_width, y + grid_height * 9);
	this->get_window()->draw_line(gc, x - grid_width, y + grid_height * 9, x + grid_width, y + grid_height * 7);
}
void MainWindow::DrawChessman()
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
