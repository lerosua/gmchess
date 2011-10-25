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
//#include "pidgin.h"

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
		void watch_socket(int fd);
		bool on_network_io(const Glib::IOCondition&);
		int init_send_socket();
		void close_send_socket();
		void send_to_socket(const std::string& cmd_);
		void set_themes(const std::string& themes_);
		void set_trace_color(const std::string& color_);


	protected:
		void on_map();
		bool on_expose_event(GdkEventExpose* ev);
		bool on_configure_event(GdkEventConfigure* ev);
		bool on_button_press_event(GdkEventButton* ev);

		Glib::RefPtr<Gdk::Pixbuf> get_pic(const std::string& name_);
		Glib::RefPtr<Gdk::Pixbuf> get_spic(const std::string& name_);

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

		/** 画选择棋子边框 */
		/** draw the frame of chess */
		void draw_select_frame(bool selected = true);
		/** show which can move in by select chess */
		void draw_show_can_move();
		/** draw prompt move point */
		void draw_phonily_point(Gdk::Point& p);
		void draw_localize(Glib::RefPtr<Gdk::GC>& gc, int x, int y, int place);
		void draw_palace(Glib::RefPtr<Gdk::GC>& gc, int x, int y);
		/** 处理拖文件事件*/
		/** handle the event of drog */
		void on_drog_data_received(const Glib::RefPtr<Gdk::DragContext>& context,
				int,int,const Gtk::SelectionData& selection_data,
				guint,guint time);
	public:

		void set_engine(const std::string& name){engine_name = name; }
		/** 启动AI对战,将会启动引擎进程*/
		/** start the AI engine*/
		void start_robot(bool new_ = true);
		/** 开启新游戏，即在引擎已启动的情况下重新游戏*/
		/** start a new game,with the presence engine*/
		void new_game(BOARD_STATUS _status = FIGHT_STATUS);
		void chanju_game(BOARD_STATUS _status = FIGHT_STATUS);
		/** 自由模式，即摆棋*/
		/** free game mode*/
		/** @param redraw 为真则重画棋盘，假则保留棋盘现状*/
		void free_game(bool redraw=true);
		/** 网络对战初始化*/
		/** network play game initial*/
		void start_network();
		/** 读取AI的输出，并根据输出的着法走棋*/
		/** read the output of AI,then go chess*/
		bool robot_log(const Glib::IOCondition& condition);
		/** 回到最初局面*/
		/** return the first station*/
		void first_move();
		/** 去到最后的局面*/
		/** go to the last station */
		void last_move();
		/** 读谱状态下走下一步棋*/
		/**  go next on reading chessbook*/
		void next_move();
		/** 读谱状态下走上一步棋*/
		/** go previous on reading chessbook */
		void back_move();
		/** 重画棋盘界面*/
		/** redraw the board*/
		void redraw();
		/** redraw the board with move trace*/
		void redraw_with_line(int mv,bool select);
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
		/** 标识是否网络对战中*/
		/** check is fight on network */
		bool is_network_game() { return m_status == NETWORK_STATUS; }
		/** 检测是不是当前用户走棋 -- check if the human user going to move */
		bool is_human_player(){ return m_engine.red_player()-m_human_black ; }
		bool is_rev_game() { return is_rev_board;}
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
		/** 设置引擎搜索深度*/
		void set_level_config(int _depth,int _idle,int _style,int _knowledge,int _pruning,int _randomness,bool _usebook);
		/** 设置对战时的走棋时间和总局时间*/
		void set_war_time(int _step_time,int _play_time);
		/** 设置保存的走棋时间和总局时间*/
		void set_time(int _step_time,int _play_time);
		/** 倒计时的声音*/
		void reckon_time_sound(int time_);
		/** 测试保存棋盘局面为图像文件*/
		void save_board_to_file(const std::string& filename);
		void draw_trace(int mv);
		
	private:
		/** 加载所需要图片进内存*/
		/** load all images in memory */
		void load_images();
	private:
		MainWindow& parent;
		/** 着法引擎 */
		Engine m_engine;
		/** UCCI engine interface*/
		Robot m_robot;
		/** 读PGN文件类*/
		Pgnfile* p_pgnfile;
		/** 传递给AI的着法状态*/
		std::string moves_lines;
		/** 着法状态开头序列，potions fen xxx*/
		const std::string postion_str;
		std::string engine_name;
		/** 所使用的主题*/
		std::string theme;
		std::string color;

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
		BOARD_STATUS m_status;
		/** 红方的局时*/
		int red_time;
		/** 黑方的局时*/
		int black_time;
		/** 用以保存和电脑下棋的局时和步时*/
		int play_time;
		int step_time;
		/** 计时，走秒*/
		int count_time;
		/** 每步时的极限秒数*/
		int limit_count_time;

		/** 接受命令的socket*/
		int fd_recv_skt;
		/** 发送命令的socket*/
		int fd_send_skt;
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
		/** 搜索深度-- search depth */
		int m_search_depth;
		/** 是否使用开局库 -- use open book*/
		bool m_usebook;
		/** 用户选择黑方棋子 -- the human choose the black player*/
		bool m_human_black;
};

#endif // _BOARD_H_

