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
	bg_image->render_to_drawable(get_window(), get_style()->get_black_gc(),
			0, 0, 0, 0, bg_image->get_width(), bg_image->get_height(), 
			Gdk::RGB_DITHER_NONE, 0, 0);
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
