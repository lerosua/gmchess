/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * Board.h
 * Copyright (C) wind 2009 <xihels@gmail.com>
 * Copyright (C) 2025 yetist <yetist@gmail.com>
 *
 */

#ifndef _BOARD_H_
#define _BOARD_H_

#include <gtk/gtk.h>
#include <iostream>
#include "gmchess.h"
#include "Engine.h"
#include "Pgnfile.h"
#include "robot.h"

class MainWindow;

struct BoardPixel {
	int x;
	int y;
	BoardPixel() : x(0), y(0) {}
	BoardPixel(int x_, int y_) : x(x_), y(y_) {}
};

class Board
{
	public:
		Board (MainWindow &win);
		~Board ();
		GtkWidget* widget() { return area; }
		const Board_info &get_board_info () { return p_pgnfile->get_board_info (); }
		void watch_socket (int fd);
		bool on_network_io (GIOCondition condition);
		int init_send_socket ();
		void close_send_socket ();
		void send_to_socket (const std::string &cmd_);
		void set_themes (const std::string &themes_);
		void set_trace_color (const std::string &color_);

	protected:
		bool on_draw(cairo_t *cr);
		bool on_button_press_event (double x, double y, guint button, int n_press);

		cairo_surface_t* get_pic (const std::string &name_);
		cairo_surface_t* get_spic (const std::string &name_);
		BoardPixel get_coordinate (int pos_x, int pos_y);
		BoardPixel get_position (int pos_x, int pos_y);
		void get_grid_size (int &width, int &height);
		void draw_bg ();
		void draw_chessman (int x, int y, int chess_type);
		void draw_surface_centered (cairo_surface_t* surface, int center_x, int center_y);
		void draw_board ();
		void draw_select_frame (bool selected = true);
		void draw_show_can_move ();
		void draw_phonily_point (BoardPixel &p);
		void draw_localize(cairo_t *cr, int x, int y, int place);
		void draw_palace(cairo_t *cr, int x, int y);
		void calcVertexes(double start_x, double start_y, double end_x, double end_y, double& x1, double& y1, double& x2, double& y2);
		void on_drop_text(const char* text);

	public:
		void set_engine (const std::string &name) { engine_name = name; }
		void start_robot (bool new_ = true);
		void new_game (BOARD_STATUS _status = FIGHT_STATUS);
		void chanju_game (BOARD_STATUS _status = FIGHT_STATUS);
		void free_game (bool redraw = true);
		void start_network ();
		bool robot_log (GIOCondition condition);
		void first_move ();
		void last_move ();
		void next_move ();
		void back_move ();
		void get_board_by_move (int num);
		int try_move (int x, int y);
		int try_move (int mv);
		void rue_move ();
		void draw_move ();
		int open_file (const std::string &filename);
		const std::vector<std::string> &get_move_chinese_snapshot () { return m_engine.get_move_chinese_snapshot (); };
		int get_status () { return m_status; }
		int get_step () { return m_step; }
		bool is_fight_to_robot () { return m_status == FIGHT_STATUS; }
		bool is_network_game () { return m_status == NETWORK_STATUS; }
		bool is_human_player () { return m_engine.red_player () - m_human_black; }
		bool is_rev_game () { return is_rev_board; }
		bool go_time ();
		std::string to_time_string (int);
		std::string to_msec_string (int);
		void set_level ();
		void configure_board (int _width);
		void rev_game ();
		void set_board_size (BOARDSIZE sizemode);
		void set_level_config (int _depth, int _idle, int _style, int _knowledge, int _pruning, int _randomness, bool _usebook);
		void set_war_time (int _step_time, int _play_time);
		void set_time (int _step_time, int _play_time);
		void reckon_time_sound (int time_);
		void save_board_to_file (const std::string &filename);
		void draw_trace (int mv);

	private:
		static void draw_cb(GtkDrawingArea* area, cairo_t* cr, int width, int height, gpointer data);
		static void button_press_cb(GtkGestureClick* gesture, int n_press, double x, double y, gpointer data);
		static gboolean drop_cb(GtkDropTarget* target, const GValue* value, double x, double y, gpointer data);
		static gboolean timer_cb(gpointer data);
		static gboolean network_io_cb(GIOChannel* source, GIOCondition condition, gpointer data);

		void load_images ();
		void queue_draw();
		void stop_timer();
		void start_timer();
		void release_images();

	private:
		MainWindow &parent;
		GtkWidget* area;
		cairo_t* active_cr;
		Engine m_engine;
		Robot m_robot;
		Pgnfile *p_pgnfile = NULL;
		std::string moves_lines;
		const std::string position_str = "position fen ";
		std::string engine_name;
		std::string theme = "wood";
		std::string color = "#198964";

		cairo_surface_t* bg_image;
		cairo_surface_t* chessman_images[18];
		int selected_x = -1;
		int selected_y = -1;
		int selected_chessman = -1;
		int m_step = 0;
		BOARD_STATUS m_status = FREE_STATUS;
		int red_time = 2400;
		int black_time = 2400;
		int play_time = 40;
		int step_time = 240;
		int count_time = 0;
		int limit_count_time = 240;

		int fd_recv_skt = -1;
		int fd_send_skt = -1;
		guint timer_id = 0;
		guint network_io_id = 0;
		int chessman_width = 29;
		bool is_small_board = true;
		bool is_rev_board = false;
		int m_search_depth = 8 ;
		bool m_usebook = true;
		bool m_human_black = false;
};

#endif // _BOARD_H_
