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

#ifndef _BOARD_H_
#define _BOARD_H_

#include <gtkmm.h>
#include <iostream>
#include <gtkmm/drawingarea.h>
#include "gmchess.h"
#include "Engine.h"


/**
 * @brief 棋盘类 
 * 负责棋盘和棋子的绘画
 */
class Board : public Gtk::DrawingArea {
	public:
		Board();
		~Board();
	protected:
		void on_map();
		bool on_expose_event(GdkEventExpose* ev);
		bool on_button_press_event(GdkEventButton* ev);

		Gdk::Point get_coordinate(int pos_x, int pos_y);
		Gdk::Point get_position(int pos_x, int pos_y);
		void get_grid_size(int& width, int& height);

		/**
		 * @brief 画棋盘背景
		 */
		void draw_bg();
		//void draw_chessman();
		/**
		 * @brief 画棋子
		 * @param x x坐标
		 * @param y y坐标
		 * @param chess_type 棋子的类型
		 */
		void draw_chessman(int x, int y, int chess_type);
		/**
		 * @brief 根据棋盘数组画出对局
		 */
		void draw_board();
		/**
		 * @brief 根据棋子数组画出对局
		 * @param pieces 棋子数组，由16->47,0表示被吃掉了
		 */
		void draw_pieces(const int pieces[]);
		void draw_select_frame(bool selected = true);
		void draw_localize(Glib::RefPtr<Gdk::GC>& gc, int x, int y, int place);
		void draw_palace(Glib::RefPtr<Gdk::GC>& gc, int x, int y);
	public:

		/** 返回棋子的类型 */
		inline int get_chessman_type(int pc) {
			  return PieceTypes[pc];
		}
#if 0
		/** 由x，y位置获得棋盘数组的位置*/
		inline int COORD_XY(int x,int y){ return x+(y<<4);};
		/** 获取y坐标*/
		inline int RANK_Y(int sq) {	  return (sq >> 4)-3;};

		/**获取x坐标*/
		inline int RANK_X(int sq) {
			  return (sq & 15)-3;
		}
#endif

		/** 读谱状态下走下一步棋*/
		void next_move();
		/** 读谱状态下走上一步棋*/
		void back_move();
	private:
		Engine m_engine;

		/** 背景图像 */
		Glib::RefPtr<Gdk::Pixbuf> bg_image;
		/** 棋子图像 */
		Glib::RefPtr<Gdk::Pixbuf> chessman_images[18];
		Glib::RefPtr<Gdk::Image> selected_chessman_image;
		int selected_x;
		int selected_y;
		int selected_chessman;
};

#endif // _BOARD_H_

