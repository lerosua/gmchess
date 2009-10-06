/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * Board.h
 * Copyright (C) wind 2009 <xihels@gmail.com>
 * 
 */

#ifndef _BOARD_H_
#define _BOARD_H_

#include <gtkmm.h>
#include <iostream>
#include <gtkmm/drawingarea.h>
#include "gmchess.h"
#include "Engine.h"
#include "Pgnfile.h"
#include "robot.h"

class MainWindow;

/**
 * @brief 棋盘类 
 * 负责棋盘和棋子的绘画
 * 棋盘9x10坐标是由左上角开始0x0- 9x10
 *
 * describe the chess board
 * draw the board and chess, the board is 9x10
 * start with 0x0 ,and with 9x10
 */
class Board : public Gtk::DrawingArea 
{
	public:
		Board(MainWindow& win);
		~Board();
		const Board_info& get_board_info(){ return p_pgnfile->get_board_info() ;}
	protected:
		void on_map();
		bool on_expose_event(GdkEventExpose* ev);
		bool on_configure_event(GdkEventConfigure* ev);
		bool on_button_press_event(GdkEventButton* ev);

		/** 由棋盘9x10坐标得到棋盘真实坐标*/
		/** get the true coordinate from 9x10 postion*/
		Gdk::Point get_coordinate(int pos_x, int pos_y);
		/** 由棋盘真实坐标得到棋盘9x10坐标*/
		/** get the 9x10 postion from the true coordinate*/
		Gdk::Point get_position(int pos_x, int pos_y);
		/** 获取棋盘格子的长与宽*/
		/** get the length and height with a chess grid */
		void get_grid_size(int& width, int& height);

		/**
		 * @brief 画棋盘背景
		 * draw the background of board
		 */
		void draw_bg();
		/**
		 * @brief 画棋子 坐标为棋盘9x10坐标
		 * draw the chess
		 * @param x x坐标
		 * @param y y坐标
		 * @param chess_type 棋子的类型,the type of chess
		 */
		void draw_chessman(int x, int y, int chess_type);
		/**
		 * @brief 根据棋盘数组画出对局
		 * draw the station of board  from chess array
		 */
		void draw_board();
		/**
		 * @brief 根据棋子数组画出对局
		 * 棋子数组，由16->47,0表示被吃掉了
		 * useless
		 */
		void draw_pieces();
		/** 画选择棋子边框 */
		/** draw the frame of chess */
		void draw_select_frame(bool selected = true);
		void draw_localize(Glib::RefPtr<Gdk::GC>& gc, int x, int y, int place);
		void draw_palace(Glib::RefPtr<Gdk::GC>& gc, int x, int y);
		/** 处理拖文件事件*/
		/** handle the event of drog */
		void on_drog_data_received(const Glib::RefPtr<Gdk::DragContext>& context,
				int,int,const Gtk::SelectionData& selection_data,
				guint,guint time);
	public:

		/** 启动AI对战,将会启动引擎进程*/
		/** start the AI engine*/
		void start_robot();
		/** 开启新游戏，即在引擎已启动的情况下重新游戏*/
		/** start a new game,with the presence engine*/
		void new_game();
		/** 自由模式，即摆棋*/
		/** free game mode*/
		void free_game();
		/** 读取AI的输出，并根据输出的着法走棋*/
		/** read the output of AI,then go chess*/
		bool robot_log(const Glib::IOCondition& condition);
		/** 回到最初局面*/
		/** return the first station*/
		void start_move();
		/** 去到最后的局面*/
		/** go to the last station */
		void end_move();
		/** 读谱状态下走下一步棋*/
		/**  go next on reading chessbook*/
		void next_move();
		/** 读谱状态下走上一步棋*/
		/** go previous on reading chessbook */
		void back_move();
		/** 重画棋盘界面*/
		/** redraw the board*/
		void redraw();
		/** 根据treeview的棋局着法获得棋盘局面*/
		/** get the board station from chess move */
		void get_board_by_move(int num);
		/** 
		 * 根据x，y坐标(棋盘9x10坐标)获取到达的坐标，与选择的棋子达成着法
		 * 进行尝试性走棋
		 */
		/** try to go move,(x,y) is the 9x10 postion */
		int try_move(int x,int y);
		/**
		 * 直接根据着法尝试性走棋
		 */
		/** try to go move, mv is chess move*/
		int try_move(int mv);
		/** 悔棋*/
		/** go rue move*/
		void rue_move();
		/** 和棋处理*/
		/** go draw move*/
		void draw_move();
		/** 打开棋谱文件*/
		/**  open the chessbook */
		int open_file(const std::string& filename);
		/** 返回中文着法表达示快照集*/
		/** return the snapshot which chinese moves*/
		const std::vector<Glib::ustring>& get_move_chinese_snapshot() {return m_engine.get_move_chinese_snapshot();};
		/** 返回程序棋盘状态:读谱，与电脑对战，网络对战,自由摆棋*/
		/** return the mode: reading,AI war,net war,free game*/
		int get_status(){ return m_status; }
		int get_step(){ return m_step; }
		/** 标识是否与电脑AI对战*/
		/** check is fight to AI */
		bool is_fight_to_robot(){ return m_status == FIGHT_STATUS;}
		bool is_human_player(){ return m_engine.red_player(); }
		/** 走时函数*/
		/** check go time*/
		bool go_time();
		/** 将数字转成时间显示*/
		/** digit convert to time show */
		Glib::ustring to_time_ustring(int);
		/** 将秒转成毫秒的string显示*/
		/** convert second to msecond show*/
		Glib::ustring to_msec_ustring(int);
		/** 设置AI等级*/
		/** set the level of AI */
		void set_level();
		void configure_board(int _width);
		/** 反转棋盘*/
		void rev_game();
		void set_board_size(BOARDSIZE sizemode);
		
	private:
		/** 加载所需要图片进内存*/
		/** load all images in memory */
		void load_images();
	private:
		MainWindow& parent;
		/** 着法引擎 */
		Engine m_engine;
		/** AI引擎*/
		Robot m_robot;
		/** 读PGN文件类*/
		Pgnfile* p_pgnfile;
		/** 传递给AI的着法状态*/
		std::string moves_lines;
		/** 着法状态开头序列，potions fen xxx*/
		const std::string postion_str;

		/** 背景图像 */
		Glib::RefPtr<Gdk::Pixbuf> bg_image;
		
		Glib::RefPtr<Gdk::Pixmap> ui_pixmap;
		/** 棋子图像 */
		Glib::RefPtr<Gdk::Pixbuf> chessman_images[18];
		/** 选中图像*/
		Glib::RefPtr<Gdk::Image> selected_chessman_image;
		/** 选中的棋盘9x10坐标*/
		int selected_x;
		int selected_y;
		/** 选中的棋子,值为代号,16-31红，32-47黑*/
		int selected_chessman;
		/** 步时 */
		int m_step;
		/** 棋局状态*/
		int m_status;
		/** 红方的局时*/
		int red_time;
		/** 黑方的局时*/
		int black_time;
		/** 计时，走秒*/
		int count_time;
		sigc::connection timer;
		/** 对战状态中标识是否用户走棋,true是用户，false是AI*/
		//bool user_player;
		/** 棋子的宽度,大的57,小的29*/
		/** width of chessman */
		int chessman_width ;
		/** 是否小棋盘*/
		bool is_small_board;
		/** 是否反转棋盘*/
		bool is_rev_board;
};

#endif // _BOARD_H_

