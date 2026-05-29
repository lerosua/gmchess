/*
 * Board.h
 * Copyright (C) wind 2009 <xihels@gmail.com>
 * Copyright (C) 2025 yetist <yetist@gmail.com>
 */

#include "Board.h"
#include "Engine.h"
#include "MainWindow.h"
#include "Sound.h"
#include "ec_throw.h"
#include "paths.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

const int border_width = 32;
const int border_height = 46;

#define PLACE_LEFT 0x01
#define PLACE_RIGHT 0x02
#define PIECE_START 16
#define PIECE_END   48
#define PLACE_ALL PLACE_LEFT | PLACE_RIGHT
#define HEX_ESCAPE '%'

int hex_to_int (gchar c);
int unescape_character (const char *scanner);
std::string  wind_unescape_string (const char *escaped_string,
    const gchar *illegal_characters);

int hex_to_int (gchar c)
{
	return  c >= '0' && c <= '9' ? c - '0'
		: c >= 'A' && c <= 'F' ? c - 'A' + 10
		: c >= 'a' && c <= 'f' ? c - 'a' + 10
		: -1;
}

int unescape_character (const char *scanner)
{
	int first_digit = hex_to_int (*scanner++);
	if (first_digit < 0)
		return -1;

	int second_digit = hex_to_int (*scanner++);
	if (second_digit < 0)
		return -1;

	return (first_digit << 4) | second_digit;
}

std::string  wind_unescape_string (const char *escaped_string,
		const gchar *illegal_characters)
{
	const char *in;
	char *out;
	int character;

	if (escaped_string == NULL)
		return std::string();

	char result[strlen (escaped_string) + 1];

	out = result;
	for (in = escaped_string; *in != '\0'; in++) {
		character = *in;
		if (*in == HEX_ESCAPE) {
			character = unescape_character (in + 1);
			if (character <= 0
					|| (illegal_characters != NULL
						&& strchr (illegal_characters, (char)character) != NULL)) {
				return std::string();
			}
			in += 2;
		}
		*out++ = (char)character;
	}

	*out = '\0';
	assert ((size_t)(out - result) <= strlen (escaped_string));
	return std::string(result);
}

void Board::draw_cb(GtkDrawingArea*, cairo_t* cr, int, int, gpointer data)
{
	static_cast<Board*>(data)->on_draw(cr);
}

void Board::button_press_cb(GtkGestureClick* gesture, int n_press, double x, double y, gpointer data)
{
	guint button = gtk_gesture_single_get_current_button(GTK_GESTURE_SINGLE(gesture));
	static_cast<Board*>(data)->on_button_press_event(x, y, button, n_press);
}

gboolean Board::drop_cb(GtkDropTarget*, const GValue* value, double, double, gpointer data)
{
	if(G_VALUE_HOLDS_STRING(value)) {
		static_cast<Board*>(data)->on_drop_text(g_value_get_string(value));
		return TRUE;
	}
	return FALSE;
}

gboolean Board::timer_cb(gpointer data)
{
	return static_cast<Board*>(data)->go_time() ? G_SOURCE_CONTINUE : G_SOURCE_REMOVE;
}

gboolean Board::network_io_cb(GIOChannel*, GIOCondition condition, gpointer data)
{
	return static_cast<Board*>(data)->on_network_io(condition) ? G_SOURCE_CONTINUE : G_SOURCE_REMOVE;
}

Board::Board(MainWindow& win)
	: parent(win)
	, area(gtk_drawing_area_new())
	, active_cr(NULL)
	, bg_image(NULL)
{
	for(int i = 0; i < 18; ++i)
		chessman_images[i] = NULL;

	gtk_widget_set_size_request(area, 221, 305);
	gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(area), draw_cb, this, NULL);

	GtkGesture* click = gtk_gesture_click_new();
	gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(click), 0);
	g_signal_connect(click, "pressed", G_CALLBACK(button_press_cb), this);
	gtk_widget_add_controller(area, GTK_EVENT_CONTROLLER(click));

	GtkDropTarget* drop_target = gtk_drop_target_new(G_TYPE_STRING, GDK_ACTION_COPY);
	g_signal_connect(drop_target, "drop", G_CALLBACK(drop_cb), this);
	gtk_widget_add_controller(area, GTK_EVENT_CONTROLLER(drop_target));

	load_images();

	p_pgnfile = new Pgnfile(m_engine);
	m_engine.init_snapshot(start_fen);
	m_robot.set_out_callback([this](GIOCondition condition) {
			return robot_log(condition);
			});
	gtk_widget_set_visible(area, TRUE);
}

Board::~Board()
{
	stop_timer();
	if(network_io_id != 0)
		g_source_remove(network_io_id);
	m_robot.send_ctrl_command("quit\n");
	m_robot.stop();
	release_images();
	delete p_pgnfile;
}

void Board::release_images()
{
	if(bg_image) {
		cairo_surface_destroy(bg_image);
		bg_image = NULL;
	}
	for(int i = 0; i < 18; ++i) {
		if(chessman_images[i]) {
			cairo_surface_destroy(chessman_images[i]);
			chessman_images[i] = NULL;
		}
	}
}

void Board::queue_draw()
{
	gtk_widget_queue_draw(area);
}

void Board::stop_timer()
{
	if(timer_id != 0) {
		g_source_remove(timer_id);
		timer_id = 0;
	}
}

void Board::start_timer()
{
	stop_timer();
	timer_id = g_timeout_add(1000, timer_cb, this);
}

void Board::set_trace_color(const std::string& color_)
{
	color = color_;
}

void Board::set_themes(const std::string& themes_)
{
	theme = themes_;
	load_images();
	queue_draw();
}

cairo_surface_t* Board::get_pic (const std::string &name_)
{
	const std::string path = std::string (DATA_DIR) +
	  "/themes/" + theme + "/" + name_;
	if(!g_file_test(path.c_str(), G_FILE_TEST_EXISTS))
		return cairo_image_surface_create_from_png(
				gmchess_data_path("themes/" + theme + "/" + name_).c_str());
	return cairo_image_surface_create_from_png(path.c_str());
}

cairo_surface_t* Board::get_spic (const std::string &name_)
{
	const std::string path = std::string (DATA_DIR) +
	  "/themes/" + theme + "-small/" + name_;
	if(!g_file_test(path.c_str(), G_FILE_TEST_EXISTS))
		return cairo_image_surface_create_from_png(
				gmchess_data_path("themes/" + theme + "-small/" + name_).c_str());
	return cairo_image_surface_create_from_png(path.c_str());
}

void Board::load_images()
{
	release_images();

	bg_image = get_pic("bg.png");
	if(is_small_board){
		chessman_images[BLACK_ADVISOR] = get_spic("black_advisor.png");
		chessman_images[BLACK_BISHOP] = get_spic("black_bishop.png");
		chessman_images[BLACK_CANNON] = get_spic("black_cannon.png");
		chessman_images[BLACK_KING] = get_spic("black_king.png");
		chessman_images[BLACK_KING_DIE] = get_spic("black_king_die.png");
		chessman_images[BLACK_KNIGHT] = get_spic("black_knight.png");
		chessman_images[BLACK_PAWN] = get_spic("black_pawn.png");
		chessman_images[BLACK_ROOK] = get_spic("black_rook.png");
		chessman_images[RED_ADVISOR] = get_spic("red_advisor.png");
		chessman_images[RED_BISHOP] = get_spic("red_bishop.png");
		chessman_images[RED_CANNON] = get_spic("red_cannon.png");
		chessman_images[RED_KING] = get_spic("red_king.png");
		chessman_images[RED_KING_DIE] = get_spic("red_king_die.png");
		chessman_images[RED_KNIGHT] = get_spic("red_knight.png");
		chessman_images[RED_PAWN] = get_spic("red_pawn.png");
		chessman_images[RED_ROOK] = get_spic("red_rook.png");
		chessman_images[SELECTED_CHESSMAN] = get_spic("select.png");
		chessman_images[PROPMT] = get_spic("null.png");
	}
	else{
		chessman_images[BLACK_ADVISOR] = get_pic("black_advisor.png");
		chessman_images[BLACK_BISHOP] = get_pic("black_bishop.png");
		chessman_images[BLACK_CANNON] = get_pic("black_cannon.png");
		chessman_images[BLACK_KING] = get_pic("black_king.png");
		chessman_images[BLACK_KING_DIE] = get_pic("black_king_die.png");
		chessman_images[BLACK_KNIGHT] = get_pic("black_knight.png");
		chessman_images[BLACK_PAWN] = get_pic("black_pawn.png");
		chessman_images[BLACK_ROOK] = get_pic("black_rook.png");
		chessman_images[RED_ADVISOR] = get_pic("red_advisor.png");
		chessman_images[RED_BISHOP] = get_pic("red_bishop.png");
		chessman_images[RED_CANNON] = get_pic("red_cannon.png");
		chessman_images[RED_KING] = get_pic("red_king.png");
		chessman_images[RED_KING_DIE] = get_pic("red_king_die.png");
		chessman_images[RED_KNIGHT] = get_pic("red_knight.png");
		chessman_images[RED_PAWN] = get_pic("red_pawn.png");
		chessman_images[RED_ROOK] = get_pic("red_rook.png");
		chessman_images[SELECTED_CHESSMAN] = get_pic("select.png");
		chessman_images[PROPMT] = get_pic("null.png");
	}
}

void Board::configure_board(int _width)
{
	if(is_small_board && _width >=521){
		is_small_board=false;
		chessman_width=57;
	}
	if(!is_small_board && _width<521){
		is_small_board=true;
		chessman_width=29;
	}
	load_images();
	queue_draw();
}

void Board::get_grid_size(int& width, int& height)
{
	width = (gtk_widget_get_width(area) - border_width * 2) / 8;
	height = (gtk_widget_get_height(area) - border_height * 2) / 9;
}

BoardPixel Board::get_coordinate(int pos_x, int pos_y)
{
	int grid_width;
	int grid_height;
	get_grid_size(grid_width, grid_height);
	pos_x = pos_x * grid_width + border_width;
	pos_y = pos_y * grid_height + border_height;
	return BoardPixel(pos_x, pos_y);
}

BoardPixel Board::get_position(int pos_x, int pos_y)
{
	int grid_width;
	int grid_height;
	get_grid_size(grid_width, grid_height);

	if(grid_width <= 0 || grid_height <= 0)
		return BoardPixel(-1, -1);

	int x = (pos_x - border_width + grid_width / 2) / grid_width;
	int y = (pos_y - border_height + grid_height / 2) / grid_height;
	if(x < 0 || x > 8 || y < 0 || y > 9)
		return BoardPixel(-1, -1);

	BoardPixel center = get_coordinate(x, y);
	const int hit_radius = std::max(chessman_width / 2, 8);
	if(std::abs(pos_x - center.x) > hit_radius ||
			std::abs(pos_y - center.y) > hit_radius)
		return BoardPixel(-1, -1);
	return BoardPixel(x, y);
}

bool Board::on_draw (cairo_t *cr)
{
	active_cr = cr;
	draw_bg();
	int mv = m_engine.get_last_move_from_snapshot();
	draw_board();
	if (mv > 0)
		draw_trace(mv);
	draw_select_frame(true);
	draw_show_can_move();
	active_cr = NULL;
	return true;
}

bool Board::on_button_press_event(double x, double y, guint button, int)
{
	if(is_fight_to_robot()||is_network_game()){
		if(!is_human_player())
			return true;
	}
	if(button == 1)
	{
		BoardPixel p = get_position((int)x, (int)y);
		selected_x = p.x;
		selected_y = p.y;
		if(selected_chessman == -1){
			CSound::play(SND_CHOOSE);
			if (selected_x != -1) {
				selected_chessman = m_engine.get_piece(selected_x, selected_y,is_rev_board);
				if(selected_chessman==0){
					selected_chessman = -1;
					printf("still no select chessman\n");
					return true;
				}
				if(is_fight_to_robot()||is_network_game()){
					if((m_human_black && (selected_chessman <32))||((!m_human_black)&&(selected_chessman>31))){
						printf("choose black %d\n",selected_chessman);
						selected_chessman =-1;
						return true;
					}
				}

				queue_draw();
			}
		}
		else{
			int dst_chessman = m_engine.get_piece(selected_x,selected_y,is_rev_board);
			if( (dst_chessman!=0) && ((selected_chessman &16)==(dst_chessman&16))){
				selected_chessman = dst_chessman;
				CSound::play(SND_CHOOSE);
				queue_draw();
			}
			else{
				try_move(selected_x,selected_y);
			}
		}
	}
	else if(button == 3){
		selected_chessman = -1;
		queue_draw();
	}
	return true;
}

void Board::draw_bg()
{
	BoardPixel p1= get_coordinate(0, 0);
	BoardPixel p2= get_coordinate(8, 9);

	int width = p2.x - p1.x;
	int height = p2.y - p1.y;

	cairo_pattern_t* pattern = cairo_pattern_create_for_surface(bg_image);
	cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REPEAT);
	cairo_set_source(active_cr, pattern);
	cairo_paint(active_cr);
	cairo_pattern_destroy(pattern);

	cairo_save(active_cr);
	cairo_set_source_rgb(active_cr, 0, 0, 0);
	cairo_set_line_width(active_cr, 4.0);
	cairo_rectangle(active_cr, p1.x - 8, p1.y - 8, width + 8 * 2, height + 8 * 2);
	cairo_stroke(active_cr);

	cairo_set_line_width(active_cr, 2.0);
	cairo_rectangle(active_cr, p1.x, p1.y, width, height);
	cairo_stroke(active_cr);

	cairo_set_line_width(active_cr, 1.0);

	for (int i = 0; i < 9; i++)
	{
		p1 = get_coordinate (0, i);
		p2 = get_coordinate (8, i);
		cairo_move_to(active_cr, p1.x, p1.y);
		cairo_line_to(active_cr, p2.x, p2.y);
		cairo_stroke(active_cr);
	}

	for (int i = 0; i < 8; i++)
	{
		p1 = get_coordinate (i, 0);
		p2 = get_coordinate (i, 4);
		cairo_move_to(active_cr, p1.x, p1.y);
		cairo_line_to(active_cr, p2.x, p2.y);
		cairo_stroke(active_cr);
	}

	for (int i = 0; i < 8; i++)
	{
		p1 = get_coordinate (i, 5);
		p2 = get_coordinate (i, 9);
		cairo_move_to(active_cr, p1.x, p1.y);
		cairo_line_to(active_cr, p2.x, p2.y);
		cairo_stroke(active_cr);
	}

	cairo_set_line_width(active_cr, 2.0);
	draw_localize (active_cr, 0, 3, PLACE_LEFT);
	draw_localize (active_cr, 8, 3, PLACE_RIGHT);

	for (int i = 0; i < 3; i++)
		draw_localize(active_cr, i * 2 + 2, 3, PLACE_ALL);

	draw_localize(active_cr, 1, 2, PLACE_ALL);
	draw_localize(active_cr, 7, 2, PLACE_ALL);

	draw_localize(active_cr, 0, 6, PLACE_LEFT);
	draw_localize(active_cr, 8, 6, PLACE_RIGHT);

	for (int i = 0; i < 3; i++)
		draw_localize(active_cr, i * 2 + 2, 6, PLACE_ALL);

	draw_localize(active_cr, 1, 7, PLACE_ALL);
	draw_localize(active_cr, 7, 7, PLACE_ALL);

	cairo_set_line_width(active_cr, 1.0);
	draw_palace (active_cr, 4, 1);
	draw_palace (active_cr, 4, 8);

	cairo_select_font_face(active_cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
			CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size(active_cr, is_small_board ? 13.0 : 15.0);
	for(int i = 0; i < 9; ++i) {
		char text[2] = { static_cast<char>('1' + i), '\0' };
		BoardPixel top = get_coordinate(i, 0);
		BoardPixel bottom = get_coordinate(i, 9);
		cairo_text_extents_t extents;

		cairo_text_extents(active_cr, text, &extents);
		cairo_move_to(active_cr, top.x - extents.width / 2.0,
				top.y - chessman_width / 2.0 - 4.0);
		cairo_show_text(active_cr, text);

		text[0] = static_cast<char>('9' - i);
		cairo_text_extents(active_cr, text, &extents);
		cairo_move_to(active_cr, bottom.x - extents.width / 2.0,
				bottom.y + chessman_width / 2.0 + 14.0);
		cairo_show_text(active_cr, text);
	}
	cairo_restore(active_cr);
}

void Board::draw_localize(cairo_t *cr, int x, int y, int place)
{
	int width;
	int height;
	get_grid_size(width, height);
	width /= 5;
	height /= 5;

	BoardPixel p = get_coordinate(x, y);

	if (place & PLACE_LEFT) {
		cairo_move_to(cr, p.x + 5, p.y - height - 4);
		cairo_line_to(cr, p.x + 5, p.y - 4);
		cairo_line_to(cr, p.x + 5 + width, p.y - 4);
		cairo_stroke(cr);

		cairo_move_to(cr, p.x + 5 + width, p.y + 5);
		cairo_line_to(cr, p.x + 5, p.y + 5);
		cairo_line_to(cr, p.x + 5, p.y + 5 + height);
		cairo_stroke(cr);
	}

	if (place & PLACE_RIGHT) {
		cairo_move_to(cr, p.x - 4 - width, p.y - 4);
		cairo_line_to(cr, p.x - 4, p.y - 4);
		cairo_line_to(cr, p.x - 4, p.y - 4 - height);
		cairo_stroke(cr);

		cairo_move_to(cr, p.x - 4 - width, p.y + 5);
		cairo_line_to(cr, p.x - 4, p.y + 5);
		cairo_line_to(cr, p.x - 4, p.y + 5 + height);
		cairo_stroke(cr);
	}
}

void Board::draw_palace(cairo_t *cr, int x, int y)
{
	int width;
	int height;
	get_grid_size(width, height);
	BoardPixel p = get_coordinate(x, y);

	cairo_move_to(cr, p.x - width, p.y - height);
	cairo_line_to(cr, p.x + width, p.y + height);
	cairo_move_to(cr, p.x + width, p.y - height);
	cairo_line_to(cr, p.x - width, p.y + height);
	cairo_stroke(cr);
}

void Board::draw_chessman(int x, int y, int chessman)
{
	int chess_type = m_engine.get_chessman_type(chessman);
	if(chess_type<0||chess_type>13)
		return;

	BoardPixel p = get_coordinate(x, y);
	draw_surface_centered(chessman_images[chess_type], p.x, p.y);
}

void Board::draw_surface_centered(cairo_surface_t* surface, int center_x, int center_y)
{
	if(!surface)
		return;

	const double px = center_x - cairo_image_surface_get_width(surface) / 2.0;
	const double py = center_y - cairo_image_surface_get_height(surface) / 2.0;
	cairo_set_source_surface(active_cr, surface, px, py);
	cairo_paint(active_cr);
}

void Board::draw_show_can_move()
{
	if(!is_fight_to_robot()&& !is_network_game())
		return;

	if (selected_chessman < 0 )
		return;
	std::vector<ChessPoint> points;
	m_engine.gen_which_can_move(points, selected_chessman, is_rev_board);

	for(auto iter = points.begin(); iter != points.end(); ++iter){
		BoardPixel p = get_coordinate(iter->x, iter->y);
		draw_phonily_point(p);
	}
}

void Board::draw_phonily_point(BoardPixel& p)
{
	draw_surface_centered(chessman_images[PROPMT], p.x, p.y);
}

void Board::draw_select_frame(bool selected)
{
	if (selected_chessman < 0 || selected_x == -1 || selected_y == -1)
		return;

	int sx,sy;
	m_engine.get_xy_from_chess(selected_chessman,sx,sy,is_rev_board);
	BoardPixel p = get_coordinate(sx, sy);

	if (selected) {
		draw_surface_centered(chessman_images[SELECTED_CHESSMAN], p.x, p.y);
	}
}

void Board::draw_board()
{
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 10; j++) {
			draw_chessman(i, j, m_engine.get_piece(i, j, is_rev_board));
		}
	}
}

void Board::calcVertexes(double start_x, double start_y, double end_x, double end_y, double& x1, double& y1, double& x2, double& y2)
{
	double arrow_lenght = chessman_width / 2.0;
	double arrow_degrees = 0.5;

	double angle = atan2 (end_y - start_y, end_x - start_x) + M_PI;

	x1 = end_x + arrow_lenght * cos(angle - arrow_degrees);
	y1 = end_y + arrow_lenght * sin(angle - arrow_degrees);
	x2 = end_x + arrow_lenght * cos(angle + arrow_degrees);
	y2 = end_y + arrow_lenght * sin(angle + arrow_degrees);
}

void Board::draw_trace(int mv)
{
	int src = m_engine.get_move_src(mv);
	int dst = m_engine.get_move_dst(mv);

	GdkRGBA rgba;
	if(!gdk_rgba_parse(&rgba, color.c_str()))
		gdk_rgba_parse(&rgba, "#198964");

	cairo_set_source_rgba(active_cr, rgba.red, rgba.green, rgba.blue, rgba.alpha);

	BoardPixel s1 = get_coordinate (m_engine.RANK_X (src) - 3, m_engine.RANK_Y (src) - 3);
	BoardPixel s2 = get_coordinate (m_engine.RANK_X (dst) - 3, m_engine.RANK_Y (dst) - 3);

	cairo_set_line_width(active_cr, 4.0);
	cairo_move_to(active_cr, s1.x, s1.y);
	cairo_line_to(active_cr, s2.x, s2.y);
	cairo_stroke(active_cr);

	double x1;
	double y1;
	double x2;
	double y2;
	calcVertexes(s1.x, s1.y, s2.x, s2.y, x1, y1, x2, y2);
	cairo_move_to(active_cr, s2.x, s2.y);
	cairo_line_to(active_cr, x1, y1);
	cairo_line_to(active_cr, x2, y2);
	cairo_close_path(active_cr);

	cairo_set_source_rgba(active_cr, rgba.red, rgba.green, rgba.blue, 0.89);
	cairo_fill(active_cr);
}

void Board::first_move()
{
	m_step = 0;
	m_engine.get_snapshot(m_step);
	std::string* str=m_engine.get_comment(m_step);
	parent.set_comment(str != NULL ? *str : " ");
	CSound::play(SND_MOVE);
	queue_draw();
}

void Board::last_move()
{
	m_step = m_engine.how_step();
	m_engine.get_snapshot(m_step);
	std::string* str=m_engine.get_comment(m_step);
	parent.set_comment(str != NULL ? *str : " ");
	CSound::play(SND_MOVE);
	queue_draw();
}

void Board::next_move()
{
	m_step++;
	int all_step = m_engine.how_step();
	if(m_step> all_step)
		m_step = all_step;
	m_engine.get_snapshot(m_step);
	std::string* str=m_engine.get_comment(m_step);
	parent.set_comment(str != NULL ? *str : " ");
	CSound::play(SND_MOVE);
	queue_draw();
}

void Board::back_move()
{
	m_step--;
	if(m_step<0)
		m_step =0;
	m_engine.get_snapshot(m_step);
	DLOG("m_step = %d\n",m_step);
	std::string* str=m_engine.get_comment(m_step);
	parent.set_comment(str != NULL ? *str : " ");
	CSound::play(SND_MOVE);
	queue_draw();
}

void Board::get_board_by_move(int f_step)
{
	int all_step = m_engine.how_step();
	if(f_step> all_step)
		f_step = all_step;
	DLOG("m_step = %d\n",f_step);

	m_engine.get_snapshot(f_step);
	m_step = f_step;
	std::string* str=m_engine.get_comment(f_step);
	parent.set_comment(str != NULL ? *str : " ");
	queue_draw();
}

int Board::try_move(int dst_x,int dst_y)
{
	int dst = m_engine.get_dst_xy(dst_x,dst_y,is_rev_board);
	int src = m_engine.get_chessman_xy(selected_chessman);
	int mv =  m_engine.get_move(src,dst);
	return try_move(mv);
}

int Board::try_move(int mv)
{
	int eat = m_engine.get_move_eat(mv);
	int dst=  m_engine.get_move_dst(mv);
	if(m_engine.make_move(mv)){
		std::string mv_chin = m_engine.get_chinese_last_move();
		int num = m_engine.how_step();
		parent.add_step_line(num,mv_chin);
		if(eat)
			CSound::play(SND_EAT);
		else
			CSound::play(SND_MOVE);

		queue_draw();
		selected_chessman = m_engine.get_piece(dst);
		printf("move = %d finish move and redraw now\n",mv);
		selected_chessman=-1;
		std::string iccs_str=m_engine.move_to_iccs_str(mv);
		if(is_fight_to_robot()){
			if(eat){
				moves_lines.clear();
				moves_lines =position_str+ m_engine.get_last_fen_from_snapshot()+std::string(" -- 0 1 ");
			}
			else{
				size_t pos = moves_lines.find("moves");
				if(pos == std::string::npos)
					moves_lines=moves_lines + " -- 0 1  moves "+iccs_str;
				else
					moves_lines=moves_lines + " "+iccs_str;
			}
			std::cout<<"moves_lines = "<<moves_lines<<std::endl;
			m_robot.send_ctrl_command(moves_lines.c_str());
			m_robot.send_ctrl_command("\n");
			if(!is_human_player()){
				char str_cmd[256];
				sprintf(str_cmd,"go depth %d \n",m_search_depth);
				m_robot.send_ctrl_command(str_cmd);
			}

			parent.change_play(is_human_player());
			count_time=0;
		}
		else if(is_network_game()){
			if(!is_human_player()){
				printf("my go\n");
				std::string mv_str = "moves:"+iccs_str;
				send_to_socket(mv_str);
			}
			parent.change_play(is_human_player());
			count_time=0;
		}
		parent.set_comment("");

		if(m_engine.mate() && is_human_player() ){
			stop_timer();
			CSound::play(SND_CHECK);
			parent.on_end_game(ROBOT_WIN);
			parent.set_comment("不要气馁，再接再大励吧!");
			DLOG("将军死棋\n");
			return 0;
		}
		if(m_engine.get_checkby()){
			CSound::play(SND_CHECK);
			DLOG("将军===============\n");
			if(is_human_player())
				parent.set_comment("您被将军了，小心噢!");
			else
				parent.set_comment("将军，干得好，看好你噢!");
		}
	}
	return 0;
}

void Board::draw_move()
{
	if(is_fight_to_robot()){
		if(is_human_player())
			m_robot.send_ctrl_command("go draw\n");
		else
			m_robot.send_ctrl_command("ponderhit draw\n");
	}
	else if(is_network_game()){
		parent.on_end_game(ROBOT_DRAW);
	}
}

void Board::rue_move()
{
	if(m_engine.how_step()<1)
		return;
	DLOG(" how_step %d\n",m_engine.how_step());
	int mv = m_engine.get_last_move_from_snapshot();
	m_engine.undo_move(mv);
	parent.del_step_last_line();

	queue_draw();

	if(is_fight_to_robot()){
		moves_lines.clear();
		moves_lines =position_str+ m_engine.get_last_fen_from_snapshot()+std::string(" -- 0 1 ");
		m_robot.send_ctrl_command(moves_lines.c_str());
		m_robot.send_ctrl_command("\n");
	}
}

int Board::open_file(const std::string& filename)
{
	if(p_pgnfile->read(filename)<0)
		return -1;
	m_step = m_engine.how_step();
	m_engine.get_snapshot(m_step);
	std::string* str=m_engine.get_comment(m_step);
	parent.set_comment(str != NULL ? *str : " ");
	m_status = READ_STATUS ;

	queue_draw();
	return 0;
}

void Board::on_drop_text(const char* text)
{
	if(!text)
		return;
	std::string filename = wind_unescape_string(text, NULL);
	size_t pos = filename.find('\r');
	if (std::string::npos != pos)
		filename = filename.substr(7, pos-7);
	if(!filename.empty())
		parent.open_file(filename);
}

void Board::free_game(bool redraw_)
{
	stop_timer();

	m_robot.send_ctrl_command("quit\n");
	m_robot.stop();
	m_status = FREE_STATUS;
	is_rev_board=false;
	m_human_black=false;

	if(redraw_){
		m_engine.init_snapshot(start_fen);
		queue_draw();
	}
}

void Board::rev_game()
{
	is_rev_board=1-is_rev_board;
	m_human_black=1-m_human_black;
	queue_draw();
}

void Board::start_robot(bool new_)
{
	m_status = FIGHT_STATUS;

	m_robot.set_engine(engine_name);
	m_robot.start();
	m_robot.send_ctrl_command("ucci\n");
	if(new_)
		new_game(m_status);
	else
		chanju_game(m_status);
}

void Board::set_level_config(int _depth,int,int,int,int,int,bool _usebook)
{
	m_search_depth = _depth;
	m_usebook = _usebook;
}

void Board::set_time(int _step_time, int _play_time)
{
	step_time = _step_time;
	play_time = _play_time;
}

void Board::set_war_time(int _step_time,int _play_time)
{
	limit_count_time = _step_time;
	red_time = _play_time*60;
	black_time = _play_time* 60;
}

void Board::set_level()
{
	m_robot.send_ctrl_command("setoption idle large\n");
	m_robot.send_ctrl_command("setoption style risky\n");
	m_robot.send_ctrl_command("setoption knowledge none\n");
	m_robot.send_ctrl_command("setoption pruning  large\n");
	m_robot.send_ctrl_command("setoption randomness large\n");
	if(m_usebook)
		m_robot.send_ctrl_command("setoption usebook false\n");
	else
		m_robot.send_ctrl_command("setoption usebook true\n");
}

void Board::start_network()
{
	set_war_time(300,30);
	new_game(NETWORK_STATUS);
}

void Board::chanju_game(BOARD_STATUS _status)
{
	m_status = _status;
	std::string cur_fen = m_engine.get_current_snapshot();
	m_engine.init_snapshot(cur_fen.c_str());

	if(m_status == FIGHT_STATUS){
		set_war_time(step_time,play_time);
		m_robot.send_ctrl_command("setoption newgame\n");
		set_level();
	}
	DLOG("current fen = %s\n", cur_fen.c_str());

	moves_lines.clear();
	moves_lines = position_str + cur_fen;
	queue_draw();

	parent.textview_engine_log_clear();
	parent.change_play(is_human_player());

	start_timer();
	if(m_human_black){
		if(m_status == FIGHT_STATUS){
			moves_lines =moves_lines +std::string(" -- 0 1 ");
			m_robot.send_ctrl_command(moves_lines.c_str());
			m_robot.send_ctrl_command("\n");
			char str_cmd[256];
			sprintf(str_cmd,"go depth %d \n",m_search_depth);
			m_robot.send_ctrl_command(str_cmd);
		}
	}

	parent.set_red_war_time(to_time_string(red_time),to_time_string(0));
	parent.set_black_war_time(to_time_string(black_time),to_time_string(0));
}

void Board::new_game(BOARD_STATUS _status)
{
	m_status = _status;

	m_engine.init_snapshot(start_fen);

	if(m_status == FIGHT_STATUS){
		set_war_time(step_time,play_time);
		m_robot.send_ctrl_command("setoption newgame\n");
		set_level();
	}

	moves_lines.clear();
	moves_lines = position_str + std::string(start_fen);
	queue_draw();

	parent.textview_engine_log_clear();
	parent.change_play(is_human_player());

	start_timer();
	if(m_human_black){
		if(m_status == FIGHT_STATUS){
			moves_lines =moves_lines +std::string(" -- 0 1 ");
			m_robot.send_ctrl_command(moves_lines.c_str());
			m_robot.send_ctrl_command("\n");
			char str_cmd[256];
			sprintf(str_cmd,"go depth %d \n",m_search_depth);
			m_robot.send_ctrl_command(str_cmd);
		}
	}

	parent.set_red_war_time(to_time_string(red_time),to_time_string(0));
	parent.set_black_war_time(to_time_string(black_time),to_time_string(0));
}

bool Board::robot_log(GIOCondition)
{
	char buf[1024] = { 0 };
	int buf_len = 1024;
	char* p = buf;
	for (; buf_len > 0; ) {
		int len = m_robot.get_robot_log(p, buf_len);
		if (len <= 0)
			break;
		p += len;
		buf_len -= len;
	}

	if (buf_len > 0) {
		*p = 0;
		printf ("%s", buf);
		std::string str_buf(buf);
		parent.show_textview_engine_log(str_buf);

		size_t pos_=str_buf.find("draw");
		if(pos_ != std::string::npos){
			printf("计算机同意和棋\n");
			if (parent.on_end_game(ROBOT_DRAW)) {
				stop_timer();
				return true;
			}
		}
		pos_=str_buf.find("resign");
		if(pos_ != std::string::npos){
			stop_timer();
			parent.on_end_game(ROBOT_LOSE);
			return true;
		}
		pos_=str_buf.find("nobestmove");
		if(pos_ != std::string::npos){
			stop_timer();
			parent.on_end_game(ROBOT_LOSE);
			return true;
		}
		size_t pos=str_buf.find("bestmove");
		if(pos != std::string::npos){
			std::string t_mv=str_buf.substr(pos+9,4);
			std::cout<<"get robot mv = "<<t_mv<<std::endl;
			int mv = m_engine.iccs_str_to_move(t_mv);
			try_move(mv);
		}
	}
	return true;
}

std::string Board::to_time_string(int ival)
{
	char sp[32];
	sprintf(sp,"%02d:%02d", ival/60,ival%60);
	return std::string(sp);
}

std::string Board::to_msec_string(int ival)
{
	char sp[32];
	sprintf(sp,"%d000",ival);
	return std::string(sp);
}

bool Board::go_time()
{
	if(is_human_player()){
		count_time++;
		red_time--;
		parent.set_red_war_time(to_time_string(red_time),to_time_string(count_time));
		if(count_time>limit_count_time-10 && count_time<= limit_count_time){
			printf("time out,you less time: %d\n",limit_count_time-count_time);
			reckon_time_sound(limit_count_time-count_time);
		}
		else if(count_time > limit_count_time || red_time<0 ){
			printf(" time limit ,you lose\n");
			stop_timer();
			parent.on_end_game(HUMAN_OVER_TIME);
			count_time =0;
			if(is_network_game())
				send_to_socket("timeout");
		}
	}
	else{
		black_time--;
		count_time++;
		parent.set_black_war_time(to_time_string(black_time),to_time_string(count_time));
		if(count_time>limit_count_time-10 && count_time<=limit_count_time){
			printf("time out,bot waster much time\n");
			m_robot.send_ctrl_command("stop\n");
		}
		else if(count_time >limit_count_time || black_time<0 ){
			printf(" time limit ,robot lose\n");
			stop_timer();
			parent.on_end_game(ROBOT_OVER_TIME);
			count_time =0;
			if(is_network_game())
				send_to_socket("timeout");
		}
	}
	return true;
}

void Board::set_board_size(BOARDSIZE sizemode)
{
	switch(sizemode){
		case BIG_BOARD:
			is_small_board=false;
			gtk_widget_set_size_request(area, 521, 605);
			break;
		case SMALL_BOARD:
			is_small_board=true;
			gtk_widget_set_size_request(area, 221, 305);
			break;
		default:
			g_warn_if_reached();
			break;
	}
}

void Board::reckon_time_sound(int time_)
{
	switch(time_){
		case 0: CSound::play(SND_0); break;
		case 1: CSound::play(SND_1); break;
		case 2: CSound::play(SND_2); break;
		case 3: CSound::play(SND_3); break;
		case 4: CSound::play(SND_4); break;
		case 5: CSound::play(SND_5); break;
		case 6: CSound::play(SND_6); break;
		case 7: CSound::play(SND_7); break;
		case 8: CSound::play(SND_8); break;
		case 9: CSound::play(SND_9); break;
		case 10: CSound::play(SND_10); break;
		default: break;
	}
}

void Board::watch_socket(int fd)
{
	fd_recv_skt=fd;
	GIOChannel* channel = g_io_channel_unix_new(fd_recv_skt);
	network_io_id = g_io_add_watch(channel, G_IO_IN, network_io_cb, this);
	g_io_channel_unref(channel);
}

bool Board::on_network_io(GIOCondition)
{
	int fd_cli = -1;
	EC_THROW(-1 == (fd_cli = accept(fd_recv_skt, NULL, 0)));
	char buf[1024];
	size_t len = read(fd_cli, &buf[0], 1023);
	buf[len]=0;
	if (len > 0) {
		std::string str_buf(buf);
		size_t pos;

		if((pos = str_buf.find("moves:")) != std::string::npos){
			std::string t_mv=str_buf.substr(pos+6,4);
			std::cout<<"get robot mv = "<<t_mv<<std::endl;
			int mv = m_engine.iccs_str_to_move(t_mv);
			try_move(mv);
		}else if((pos = str_buf.find("network-game-red,"))!= std::string::npos){
			std::string enemy_name,my_name;
			size_t pos_s,pos_e,pos_m;
			pos_s= str_buf.find("enemy_name:");
			pos_e= str_buf.find(",my_name:");
			pos_m= str_buf.find_first_of("@");
			enemy_name = str_buf.substr(pos_s+11,pos_m-pos_s-11);
			pos_m = str_buf.find_last_of("@");
			my_name = str_buf.substr(pos_e+9,pos_m-pos_e-9);
			parent.on_network_game(enemy_name,my_name,true);
		}else if((pos = str_buf.find("network-game-black,")) != std::string::npos){
			std::string enemy_name,my_name;
			size_t pos_s,pos_e,pos_m;
			pos_s= str_buf.find("enemy_name:");
			pos_e= str_buf.find(",my_name:");
			pos_m= str_buf.find_first_of("@");
			enemy_name = str_buf.substr(pos_s+11,pos_m-pos_s-11);
			pos_m = str_buf.find_last_of("@");
			my_name = str_buf.substr(pos_e+9,pos_m-pos_e-9);
			parent.on_network_game(my_name,enemy_name,false);
		}else if(str_buf.find("network-game-win") != std::string::npos){
			stop_timer();
			parent.on_end_game(ROBOT_LOSE);
			return true;
		}else if(str_buf.find("resign") != std::string::npos){
			stop_timer();
			parent.on_end_game(ROBOT_WIN);
			return true;
		}else if(str_buf.find("network-game-rue") != std::string::npos){
			rue_move();
			rue_move();
		}else if(str_buf.find("network-game-norue") != std::string::npos){
			parent.info_window("The against doesn't agree rue!");
		}else if(str_buf.find("network-game-nodraw") != std::string::npos){
			parent.info_window(("The against doesn't agree draw!"));
		}else if(str_buf.find("network-game-draw") != std::string::npos){
			stop_timer();
			parent.on_end_game(ROBOT_DRAW);
		}
	}
	close(fd_cli);
	return true;
}

int Board::init_send_socket()
{
	int sockfd;
	struct sockaddr_in srvaddr;

	EC_THROW(-1 == (sockfd=socket(AF_INET,SOCK_STREAM,0)));
	memset(&srvaddr,0,sizeof(srvaddr));
	srvaddr.sin_family=AF_INET;
	srvaddr.sin_port=htons(GMPORT+1);
	srvaddr.sin_addr.s_addr=htonl(INADDR_ANY);

	int on = 1;
	EC_THROW( -1 == (setsockopt( sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) )));

	return sockfd;
}

void Board::send_to_socket(const std::string& cmd_)
{
	int sockfd;
	struct sockaddr_in srvaddr;

	EC_THROW(-1 == (sockfd=socket(AF_INET,SOCK_STREAM,0)));
	memset(&srvaddr,0,sizeof(srvaddr));
	srvaddr.sin_family=AF_INET;
	srvaddr.sin_port=htons(GMPORT+1);
	srvaddr.sin_addr.s_addr=htonl(INADDR_ANY);

	int on = 1;
	EC_THROW( -1 == (setsockopt( sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) )));

	if( 0 == connect(sockfd,(struct sockaddr*)&srvaddr,sizeof(srvaddr))){
		ssize_t written = write(sockfd,cmd_.c_str(),cmd_.size());
		(void)written;
		close(sockfd);
	}
}

void Board::close_send_socket()
{
	if(fd_send_skt>0){
		close(fd_send_skt);
		fd_send_skt=-1;
	}
}

void Board::save_board_to_file(const std::string& filename)
{
	const int width = gtk_widget_get_width(area);
	const int height = gtk_widget_get_height(area);
	cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
	cairo_t* cr = cairo_create(surface);
	active_cr = cr;
	draw_bg();
	draw_board();
	int mv = m_engine.get_last_move_from_snapshot();
	if (mv > 0)
		draw_trace(mv);
	draw_select_frame(true);
	draw_show_can_move();
	active_cr = NULL;
	cairo_destroy(cr);
	cairo_surface_write_to_png(surface, filename.c_str());
	cairo_surface_destroy(surface);
}
