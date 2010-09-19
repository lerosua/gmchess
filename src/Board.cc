/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * Board.h
 * Copyright (C) wind 2009 <xihels@gmail.com>
 * 
 */

#include "Board.h"
#include <vector>
#include <string.h>
#include <cassert>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include "Engine.h"
#include "MainWindow.h"
#include "Sound.h"
#include "ec_throw.h"

/** 边界的宽度*/
/**  width of border */
const int border_width = 32;
/** 棋子的宽度*/
/** width of chessman */
//const int chessman_width = 57;
//const int chessman_width = 29;

#define	PLACE_LEFT 0x01
#define PLACE_RIGHT 0x02
#define PIECE_START 16  //棋子开始数字 start number of chessman
#define PIECE_END   48  //棋子结束数字 end number of chessman
#define PLACE_ALL PLACE_LEFT | PLACE_RIGHT
#define IMGAGE_DIR DATA_DIR"/wood/"
#define IMGAGE_SMALL_DIR DATA_DIR"/wood-small/"
#define HEX_ESCAPE '%'


int hex_to_int (gchar c)
{
	return  c >= '0' && c <= '9' ? c - '0'
		: c >= 'A' && c <= 'F' ? c - 'A' + 10
		: c >= 'a' && c <= 'f' ? c - 'a' + 10
		: -1;
}

int unescape_character (const char *scanner)
{
	int first_digit;
	int second_digit;

	first_digit = hex_to_int (*scanner++);
	if (first_digit < 0) {
		return -1;
	}

	second_digit = hex_to_int (*scanner++);
	if (second_digit < 0) {
		return -1;
	}

	return (first_digit << 4) | second_digit;
}

/** 用于在拖放时得到的文件名的转码*/
/** get the code of when drag the filename*/
std::string  wind_unescape_string (const char *escaped_string, 
		const gchar *illegal_characters)
{
	const char *in;
	char *out;
	int character;

	if (escaped_string == NULL) {
		return std::string();
	}

	//result = g_malloc (strlen (escaped_string) + 1);
	char result[strlen (escaped_string) + 1];

	out = result;
	for (in = escaped_string; *in != '\0'; in++) {
		character = *in;
		if (*in == HEX_ESCAPE) {
			character = unescape_character (in + 1);

			/* Check for an illegal character. We consider '\0' illegal here. */
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
	assert (out - result <= strlen (escaped_string));
	return std::string(result);

}

Board::Board(MainWindow& win) :
	selected_x(-1),
	selected_y(-1),
	m_step(0),
	m_status(FREE_STATUS),
	ui_pixmap(NULL),
	p_pgnfile(NULL),
	fd_recv_skt(-1),
	fd_send_skt(-1),
	selected_chessman(-1)
	,postion_str("position fen ")
	,parent(win)
	,red_time(2400)
	,black_time(2400)
	,count_time(0)
	,limit_count_time(240)
	,chessman_width(29)
	,is_small_board(true)
	,is_rev_board(false)
	,m_search_depth(8)
	,m_usebook(true)
	,m_human_black(false)
{

	std::list<Gtk::TargetEntry> listTargets;
	listTargets.push_back(Gtk::TargetEntry("STRING"));
	listTargets.push_back(Gtk::TargetEntry("text/plain"));


	this->set_size_request(221,277);
	//this->set_size_request(521,577);

	this->drag_dest_set(listTargets);
	this->signal_drag_data_received().connect(
			sigc::mem_fun(*this, &Board::on_drog_data_received));

	/** 加载所需要图片进内存*/
	load_images();
	
	p_pgnfile=new Pgnfile(m_engine);
	m_engine.init_snapshot(start_fen);
	m_robot.set_out_slot(sigc::mem_fun(*this, &Board::robot_log));
	//m_network.set_out_slot(sigc::mem_fun(*this, &Board::network_log));
	this->set_events(Gdk::BUTTON_PRESS_MASK|Gdk::EXPOSURE_MASK);

	this->show_all();



}

Board::~Board()
{
	if(timer.connected())
		timer.disconnect();
	m_robot.send_ctrl_command("quit\n");
	m_robot.stop();
	//m_network.stop();
}

void Board::load_images()
{
	bg_image = Gdk::Pixbuf::create_from_file(IMGAGE_DIR"bg.png");

	if(is_small_board){
		chessman_images[BLACK_ADVISOR] = Gdk::Pixbuf::create_from_file(IMGAGE_SMALL_DIR"black_advisor.png");
		chessman_images[BLACK_BISHOP] = Gdk::Pixbuf::create_from_file(IMGAGE_SMALL_DIR"black_bishop.png");
		chessman_images[BLACK_CANNON] = Gdk::Pixbuf::create_from_file(IMGAGE_SMALL_DIR"black_cannon.png");
		chessman_images[BLACK_KING] = Gdk::Pixbuf::create_from_file(IMGAGE_SMALL_DIR"black_king.png");
		chessman_images[BLACK_KING_DIE] = Gdk::Pixbuf::create_from_file(IMGAGE_SMALL_DIR"black_king_die.png");
		chessman_images[BLACK_KNIGHT] = Gdk::Pixbuf::create_from_file(IMGAGE_SMALL_DIR"black_knight.png");
		chessman_images[BLACK_PAWN] = Gdk::Pixbuf::create_from_file(IMGAGE_SMALL_DIR"black_pawn.png");
		chessman_images[BLACK_ROOK] = Gdk::Pixbuf::create_from_file(IMGAGE_SMALL_DIR"black_rook.png");
		chessman_images[RED_ADVISOR] = Gdk::Pixbuf::create_from_file(IMGAGE_SMALL_DIR"red_advisor.png");
		chessman_images[RED_BISHOP] = Gdk::Pixbuf::create_from_file(IMGAGE_SMALL_DIR"red_bishop.png");
		chessman_images[RED_CANNON] = Gdk::Pixbuf::create_from_file(IMGAGE_SMALL_DIR"red_cannon.png");
		chessman_images[RED_KING] = Gdk::Pixbuf::create_from_file(IMGAGE_SMALL_DIR"red_king.png");
		chessman_images[RED_KING_DIE] = Gdk::Pixbuf::create_from_file(IMGAGE_SMALL_DIR"red_king_die.png");
		chessman_images[RED_KNIGHT] = Gdk::Pixbuf::create_from_file(IMGAGE_SMALL_DIR"red_knight.png");
		chessman_images[RED_PAWN] = Gdk::Pixbuf::create_from_file(IMGAGE_SMALL_DIR"red_pawn.png");
		chessman_images[RED_ROOK] = Gdk::Pixbuf::create_from_file(IMGAGE_SMALL_DIR"red_rook.png");
		chessman_images[SELECTED_CHESSMAN] = Gdk::Pixbuf::create_from_file(IMGAGE_SMALL_DIR"select.png");
		chessman_images[NULL_CHESSMAN] = Gdk::Pixbuf::create_from_file(IMGAGE_SMALL_DIR"null.png");
	}
	else{

		chessman_images[BLACK_ADVISOR] = Gdk::Pixbuf::create_from_file(IMGAGE_DIR"black_advisor.png");
		chessman_images[BLACK_BISHOP] = Gdk::Pixbuf::create_from_file(IMGAGE_DIR"black_bishop.png");
		chessman_images[BLACK_CANNON] = Gdk::Pixbuf::create_from_file(IMGAGE_DIR"black_cannon.png");
		chessman_images[BLACK_KING] = Gdk::Pixbuf::create_from_file(IMGAGE_DIR"black_king.png");
		chessman_images[BLACK_KING_DIE] = Gdk::Pixbuf::create_from_file(IMGAGE_DIR"black_king_die.png");
		chessman_images[BLACK_KNIGHT] = Gdk::Pixbuf::create_from_file(IMGAGE_DIR"black_knight.png");
		chessman_images[BLACK_PAWN] = Gdk::Pixbuf::create_from_file(IMGAGE_DIR"black_pawn.png");
		chessman_images[BLACK_ROOK] = Gdk::Pixbuf::create_from_file(IMGAGE_DIR"black_rook.png");
		chessman_images[RED_ADVISOR] = Gdk::Pixbuf::create_from_file(IMGAGE_DIR"red_advisor.png");
		chessman_images[RED_BISHOP] = Gdk::Pixbuf::create_from_file(IMGAGE_DIR"red_bishop.png");
		chessman_images[RED_CANNON] = Gdk::Pixbuf::create_from_file(IMGAGE_DIR"red_cannon.png");
		chessman_images[RED_KING] = Gdk::Pixbuf::create_from_file(IMGAGE_DIR"red_king.png");
		chessman_images[RED_KING_DIE] = Gdk::Pixbuf::create_from_file(IMGAGE_DIR"red_king_die.png");
		chessman_images[RED_KNIGHT] = Gdk::Pixbuf::create_from_file(IMGAGE_DIR"red_knight.png");
		chessman_images[RED_PAWN] = Gdk::Pixbuf::create_from_file(IMGAGE_DIR"red_pawn.png");
		chessman_images[RED_ROOK] = Gdk::Pixbuf::create_from_file(IMGAGE_DIR"red_rook.png");
		chessman_images[SELECTED_CHESSMAN] = Gdk::Pixbuf::create_from_file(IMGAGE_DIR"select.png");
		chessman_images[NULL_CHESSMAN] = Gdk::Pixbuf::create_from_file(IMGAGE_DIR"null.png");

	}
	
}

void Board::configure_board(int _width)
{
	if(is_small_board && _width >=521){
		is_small_board=false;
		chessman_width=57;
		load_images();
		redraw();

	}
	if(!is_small_board && _width<521){
		is_small_board=true;
		chessman_width=29;
		load_images();
		redraw();
	}
	
}

void Board::get_grid_size(int& width, int& height)
{
	width = (get_width() - border_width * 2) / 8;
	height = (get_height() - border_width * 2) / 9;
}

Gdk::Point Board::get_coordinate(int pos_x, int pos_y)
{
	int grid_width;
	int grid_height;
	get_grid_size(grid_width, grid_height);
	pos_x = pos_x * grid_width + border_width;
	pos_y = pos_y * grid_height + border_width;

	return Gdk::Point(pos_x, pos_y);
}


Gdk::Point Board::get_position(int pos_x, int pos_y)
{
	int grid_width;
	int grid_height;
	get_grid_size(grid_width, grid_height);
	pos_x -= border_width;
	pos_y -= border_width;
	pos_x += grid_width / 2;
	pos_y += grid_height / 2;
	int x = pos_x / grid_width;
	int y = pos_y / grid_height;
	int offset_x = pos_x - x * grid_width;
	int offset_y = pos_y - y * grid_height;
	if ((offset_x > chessman_width ) || (offset_y > chessman_width ))
		return Gdk::Point(-1, -1);
	return Gdk::Point(x, y);
}

void Board::on_map()
{
	Gtk::DrawingArea::on_map();
	selected_chessman_image = Gdk::Image::create (Gdk::IMAGE_SHARED, get_window()->get_visual(), chessman_width, chessman_width);
}

bool Board::on_configure_event(GdkEventConfigure* ev)
{
	//if(ui_pixmap)
	//	return true;
	ui_pixmap = Gdk::Pixmap::create(this->get_window(),get_width(),get_height());

	redraw();

}
bool Board::on_expose_event(GdkEventExpose* ev)
{
	this->get_window()->draw_drawable(this->get_style()->get_black_gc(),ui_pixmap,
			ev->area.x,ev->area.y,
			ev->area.x,ev->area.y,
			ev->area.width,ev->area.height);
			
	return true;
}
void Board::redraw()
{
	draw_bg();
	draw_board();

	int x,y;
	ui_pixmap->get_size(x,y);
	this->get_window()->draw_drawable(this->get_style()->get_black_gc(),ui_pixmap,
			0,0,
			0,0,
			x,y);
}

/**处理点击事件, handle the click events*/
bool Board::on_button_press_event(GdkEventButton* ev)
{
	/** 对战状态下，电脑走棋时就不响应鼠标事件了*/
	/** if fight with AI, the compute doesn't handle the events */
	if(is_fight_to_robot()||is_network_game()){
		if(!is_human_player())
			return true;

	}
	if(ev->type == GDK_BUTTON_PRESS&& ev->button == 1)
	{
		redraw();
		//draw_select_frame(false);
		Gdk::Point p = get_position(ev->x, ev->y);
		selected_x = p.get_x();
		selected_y = p.get_y();
		if(selected_chessman == -1){
			/** 之前没选中棋子，现在选择 */
			/** there is not select chessman,now select*/

			CSound::play(SND_CHOOSE);
			if (selected_x != -1) {
				selected_chessman = m_engine.get_piece(selected_x, selected_y,is_rev_board);
				if(selected_chessman==0){
					/** 仍然没选中, still not select */
					selected_chessman = -1;
					printf("still no select chessman\n");
					return true;
				}
				/** 对战状态中，选了对方棋子无效*/
				/** choose the enemy chessman is useless on war */
				if(is_fight_to_robot()||is_network_game()){ 
					if((m_human_black && (selected_chessman <32))||((!m_human_black)&&(selected_chessman>31))){
						printf("choose black %d\n",selected_chessman);
						selected_chessman =-1;
						//draw_select_frame(false); 
						return true;
					}
				}

				draw_select_frame(true);
			}
		}
		else{
			/** 之前已经选中棋子，现在是生成着法或取消*/
			/** there has a selecter, now buider the moves or canel */
			int dst_chessman = m_engine.get_piece(selected_x,selected_y,is_rev_board);
			if( (dst_chessman!=0) && ((selected_chessman &16)==(dst_chessman&16))){
				/** 之前所选及现在选是同一色棋子, 变更棋子选择 */
				/** change the select */
				selected_chessman = dst_chessman;
				draw_select_frame(true);
				CSound::play(SND_CHOOSE);

			}
			else if(dst_chessman == 0){
				/** 目标地点没有棋子可以直接生成着法，当然还需要检测一下从源地点到终点是否是合法的着法，交由下面着法生成函数负责*/
				try_move(selected_x,selected_y);
			}
			else{
				/** 目标地点有对方棋子，其实也可以给着法生成函数搞啊*/
				try_move(selected_x,selected_y);
			}


		}
	}
	else if(ev->type == GDK_BUTTON_PRESS&& ev->button == 3){
		/** 右键取消选择*/
		/** the right click canel the choose*/
		selected_chessman = -1;
		//draw_select_frame(false);
		redraw();

	}

	return true;
}

void Board::draw_bg()
{
	Gdk::Point p1= get_coordinate(0, 0);
	Gdk::Point p2= get_coordinate(8, 9);

	int width = p2.get_x() - p1.get_x();
	int height = p2.get_y() - p1.get_y();

	int bg_width = bg_image->get_width();
	int bg_height = bg_image->get_height();

	int count_w = get_width() / bg_width + 1;
	int count_h = get_height() / bg_height + 1;
	for (int i = 0; i < count_w; i++) {
		for (int j = 0; j < count_h; j++) {
			ui_pixmap->draw_pixbuf(get_style()->get_black_gc(), bg_image,
					0, 0, i * bg_width, j * bg_height, bg_width, bg_height, 
					Gdk::RGB_DITHER_NONE, 0, 0);
		}
	}


	Glib::RefPtr<Gdk::GC> gc = this->get_style()->get_black_gc();
	gc->set_line_attributes(4, Gdk::LINE_SOLID, Gdk::CAP_NOT_LAST, Gdk::JOIN_ROUND);
	ui_pixmap->draw_rectangle(gc, false, 
			p1.get_x() - 8, p1.get_y() - 8,
			width + 8 * 2, height + 8 * 2);

	gc->set_line_attributes(2, Gdk::LINE_SOLID, Gdk::CAP_NOT_LAST, Gdk::JOIN_ROUND);
	ui_pixmap->draw_rectangle(gc, false, 
			p1.get_x(), p1.get_y(),
			width, height);

	gc->set_line_attributes(1, Gdk::LINE_SOLID, Gdk::CAP_NOT_LAST, Gdk::JOIN_ROUND);
	int grid_width;
	int grid_height;
	get_grid_size(grid_width, grid_height);

	GdkSegment seg[9];

	for (int i = 0; i < 9; i++) {
		p1 = get_coordinate(0, i);
		p2 = get_coordinate(8, i);
		seg[i].x1 = p1.get_x();
		seg[i].y1 = p1.get_y();
		seg[i].x2 = p2.get_x();
		seg[i].y2 = p2.get_y();
	}
	ui_pixmap->draw_segments(gc, seg, 9);

	for (int i = 0; i < 8; i++) {
		p1 = get_coordinate(i, 0);
		p2 = get_coordinate(i, 4);
		seg[i].x1 = p1.get_x();
		seg[i].y1 = p1.get_y();
		seg[i].x2 = p2.get_x();
		seg[i].y2 = p2.get_y();
	}
	ui_pixmap->draw_segments(gc, seg, 8);

	for (int i = 0; i < 8; i++) {
		p1 = get_coordinate(i, 5);
		p2 = get_coordinate(i, 9);
		seg[i].x1 = p1.get_x();
		seg[i].y1 = p1.get_y();
		seg[i].x2 = p2.get_x();
		seg[i].y2 = p2.get_y();
	}
	ui_pixmap->draw_segments(gc, seg, 8);

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

void Board::draw_localize(Glib::RefPtr<Gdk::GC>& gc, int x, int y, int place)
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
		ui_pixmap->draw_lines(gc, poss);

		poss.clear();
		poss.push_back(Gdk::Point(p.get_x() + 5 + width, p.get_y() + 5));
		poss.push_back(Gdk::Point(p.get_x() + 5, p.get_y() + 5));
		poss.push_back(Gdk::Point(p.get_x() + 5 , p.get_y() + 5 + height));
		ui_pixmap->draw_lines(gc, poss);
	}

	if (place & PLACE_RIGHT) {
		std::vector<Gdk::Point> poss;
		poss.push_back(Gdk::Point(p.get_x() - 4 - width, p.get_y() - 4));
		poss.push_back(Gdk::Point(p.get_x() - 4 , p.get_y() - 4));
		poss.push_back(Gdk::Point(p.get_x() - 4 , p.get_y() - 4 - height));
		ui_pixmap->draw_lines(gc, poss);
		poss.clear();
		poss.push_back(Gdk::Point(p.get_x() - 4 - width, p.get_y() + 5));
		poss.push_back(Gdk::Point(p.get_x() - 4 , p.get_y() + 5));
		poss.push_back(Gdk::Point(p.get_x() - 4 , p.get_y() + 5 + height));
		ui_pixmap->draw_lines(gc, poss);
	}
}

void Board::draw_palace(Glib::RefPtr<Gdk::GC>& gc, int x, int y)
{
	int width;
	int height;
	get_grid_size(width, height);
	Gdk::Point p = get_coordinate(x, y);

	ui_pixmap->draw_line(gc, p.get_x() - width, p.get_y() - height, p.get_x() + width, p.get_y() + height);
	ui_pixmap->draw_line(gc, p.get_x() + width, p.get_y() - height, p.get_x() - width, p.get_y() + height);
}


void Board::draw_chessman(int x, int y, int chessman)
{
	//if(chessman<16|| chessman>47)
	//	return;
	int chess_type = m_engine.get_chessman_type(chessman);	
	if(chess_type<0||chess_type>13)
		return;

	Gdk::Point p = get_coordinate(x, y);
	int px = p.get_x() - chessman_width / 2;
	int py = p.get_y() - chessman_width / 2;

	ui_pixmap->draw_pixbuf(get_style()->get_black_gc(),chessman_images[chess_type],
			0, 0, px, py, chessman_images[chess_type]->get_width(), chessman_images[chess_type]->get_height(), 
			Gdk::RGB_DITHER_NONE, 0, 0);
}

void Board::draw_select_frame(bool selected)
{
	if (selected_chessman < 0 || selected_x == -1 || selected_y == -1)
		return;

	/** 目前要做的是根据棋子代号，获取它所在的棋盘9x10坐标*/
	//Gdk::Point p = get_coordinate(selected_x, selected_y);
	int sx,sy;
	m_engine.get_xy_from_chess(selected_chessman,sx,sy,is_rev_board);
	Gdk::Point p = get_coordinate(sx, sy);


	int px = p.get_x() - chessman_width / 2;
	int py = p.get_y() - chessman_width / 2;


	if (selected) {
		ui_pixmap->copy_to_image(selected_chessman_image, px, py, 0, 0, chessman_width, chessman_width);
	ui_pixmap->draw_pixbuf(get_style()->get_black_gc(),chessman_images[SELECTED_CHESSMAN],
				0, 0, px, py, chessman_images[SELECTED_CHESSMAN]->get_width(), chessman_images[SELECTED_CHESSMAN]->get_height(), 
				Gdk::RGB_DITHER_NONE, 0, 0);
	} else {
		ui_pixmap->draw_image (get_style()->get_black_gc(), selected_chessman_image, 0, 0, px, py, -1, -1);
	}


	int x,y;
	ui_pixmap->get_size(x,y);
	this->get_window()->draw_drawable(this->get_style()->get_black_gc(),ui_pixmap,
			0,0,
			0,0,
			x,y);

}

void Board::draw_pieces()
{
#if 0
	int i,j;
	i = PIECE_START;
	for(;i<PIECE_END;i++)
	{
		int sq = pieces[i];
		if(sq == 0||sq<0x33||sq>0xcb)
			continue;
		draw_chessman(RANK_X(sq),RANK_Y(sq),get_chessman_type(i));
	}
	draw_select_frame(true);
#endif
}

void Board::draw_board()
{
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 10; j++) {
			draw_chessman(i, j, m_engine.get_piece(i, j,is_rev_board));
		}
	}
}

void Board::first_move()
{
	m_step = 0;
	m_engine.get_snapshot(m_step);

	/** 设置此步的注释*/
	/** set the comment of this  move*/
	std::string* str=m_engine.get_comment(m_step);
	if(str != NULL){
		parent.set_comment(*str);
	}
	else
		parent.set_comment(" ");

	CSound::play(SND_MOVE);
	redraw();
}

void Board::last_move()
{
	m_step = m_engine.how_step();
	m_engine.get_snapshot(m_step);

	/** 设置此步的注释*/
	/** set the comment of this  move*/
	std::string* str=m_engine.get_comment(m_step);
	if(str != NULL){
		parent.set_comment(*str);
	}
	else
		parent.set_comment(" ");

	CSound::play(SND_MOVE);
	redraw();
}
void Board::next_move()
{
	m_step++;
	int all_step = m_engine.how_step();
	if(m_step> all_step)
		m_step = all_step;
	m_engine.get_snapshot(m_step);

	/** 设置此步的注释*/
	/** set the comment of this  move*/
	std::string* str=m_engine.get_comment(m_step);
	if(str != NULL){
		parent.set_comment(*str);
	}
	else
		parent.set_comment(" ");

	CSound::play(SND_MOVE);
	redraw();
}
void Board::back_move()
{
	
	m_step--;
	if(m_step<0)
		m_step =0;
	m_engine.get_snapshot(m_step);
	DLOG("m_step = %d\n",m_step);


	/** 设置此步的注释*/
	/** set the comment of this  move*/
	std::string* str=m_engine.get_comment(m_step);
	if(str != NULL){
		parent.set_comment(*str);
	}
	else
		parent.set_comment(" ");

	CSound::play(SND_MOVE);
	redraw();

}

void Board::get_board_by_move(int f_step)
{
	//m_step = (num+1)*2;
	int all_step = m_engine.how_step();
	if(f_step> all_step)
		f_step = all_step;
	DLOG("m_step = %d\n",f_step);

	m_engine.get_snapshot(f_step);
	m_step = f_step;

	/** 设置此步的注释*/
	/** set the comment of this  move*/
	std::string* str=m_engine.get_comment(f_step);
	if(str != NULL){
		parent.set_comment(*str);
	}
	else
		parent.set_comment(" ");
	redraw();
}

int Board::try_move(int dst_x,int dst_y)
{

	int dst = m_engine.get_dst_xy(dst_x,dst_y,is_rev_board);
	int src = m_engine.get_chessman_xy(selected_chessman);
	int mv =  m_engine.get_move(src,dst);
	int eat = m_engine.get_move_eat(mv);
	DLOG("Board:: src = %x dst = %x mv = %d eat = %d\n",src,dst,mv,eat);
	return try_move(mv);
}
int Board::try_move(int mv)
{
	int eat = m_engine.get_move_eat(mv);
	int dst=  m_engine.get_move_dst(mv);
	/** 对着法进行逻辑检测*/
	/** check the logic of the move */
	if(m_engine.make_move(mv)){
		/** 执行着法*/
		//m_engine.do_move(mv);
		/** 将着法中文表示加到treeview中*/
		/**  and the chinese moves to treeview */
		Glib::ustring mv_chin = m_engine.get_chinese_last_move();
		int num = m_engine.how_step();
		parent.add_step_line(num,mv_chin);
		if(eat)
			CSound::play(SND_EAT);
		else
			CSound::play(SND_MOVE);

		redraw();
		selected_chessman = m_engine.get_piece(dst);
		draw_select_frame(true);
		printf("sleceted = %d finish move and redraw now\n",selected_chessman);
		selected_chessman=-1;

		std::string iccs_str=m_engine.move_to_iccs_str(mv);
		/** 对战时的处理*/
		if(is_fight_to_robot()){
			if(eat){
				moves_lines.clear();
				moves_lines =postion_str+ m_engine.get_last_fen_from_snapshot()+std::string(" -- 0 1 ");
			}
			else{
				size_t pos = moves_lines.find("moves");
				if(pos == std::string::npos){
					moves_lines=moves_lines + " -- 0 1  moves "+iccs_str;
				}else{
					moves_lines=moves_lines + " "+iccs_str;
				}

			}
			/** then send the moves_lines to ucci engine(robot)*/
			std::cout<<"moves_lines = "<<moves_lines<<std::endl;
			m_robot.send_ctrl_command(moves_lines.c_str());
			m_robot.send_ctrl_command("\n");
			//user_player = 1-user_player;
			if(!is_human_player()){
				//Glib::ustring str_cmd="go time "+to_msec_ustring(black_time)+" increment 0\n";
				//Glib::ustring str_cmd=Glib::ustring("go depth ")+search_depth+"\n";
				char str_cmd[256];
				sprintf(str_cmd,"go depth %d \n",m_search_depth);
				m_robot.send_ctrl_command(str_cmd);
				//m_robot.send_ctrl_command("go time 295000 increment 0\n");
			}


			parent.change_play(is_human_player());
			count_time=0;
		}
		else if(is_network_game()){

			if(!is_human_player()){
			/** 我走的棋，则将iccs_str走法传给网络*/
				printf("my go\n");
				std::string mv_str = "moves:"+iccs_str;
				send_to_socket(mv_str);


			}
			parent.change_play(is_human_player());
			count_time=0;

		}

		/**被将死了*/
		/** is it  mate */
		if(m_engine.mate() && is_human_player() ){
			if(timer.connected())
				timer.disconnect();
			CSound::play(SND_CHECK);
			parent.on_end_game(ROBOT_WIN);
			DLOG("将军死棋\n");
			return 0;
		}
		if(m_engine.get_checkby()){
			CSound::play(SND_CHECK);
			DLOG("将军===============\n");
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
	//DLOG(" undo move %d\n");
	m_engine.undo_move(mv);
	parent.del_step_last_line();

	redraw();

	if(is_fight_to_robot()){
		moves_lines.clear();
		moves_lines =postion_str+ m_engine.get_last_fen_from_snapshot()+std::string(" -- 0 1 ");
		m_robot.send_ctrl_command(moves_lines.c_str());
		m_robot.send_ctrl_command("\n");
		//user_player = 1-user_player;
	}
}


int Board::open_file(const std::string& filename)
{

	/** for test pgnfile */
	if(p_pgnfile->read(filename)<0){
		/** open fail */
		return -1;
	}
	m_step = m_engine.how_step();
	m_status = READ_STATUS ;

	redraw();
	return 0;
}


void Board::on_drog_data_received(const Glib::RefPtr<Gdk::DragContext>& context,
		int, int, const Gtk::SelectionData& selection_data,
		guint,guint f_time)
{
	if((selection_data.get_length() >= 0)&&(selection_data.get_format()== 8))
	{
		context->drag_finish(false,false,f_time);
		std::string filename = wind_unescape_string(selection_data.get_text().c_str(), NULL);
		size_t pos = filename.find('\r');
		if (std::string::npos != pos)
			filename = filename.substr(7, pos-7);
		parent.open_file(filename);

	}
}

void Board::free_game(bool redraw_)
{
	if(timer.connected())
		timer.disconnect();

	m_robot.send_ctrl_command("quit\n");
	m_robot.stop();
	//m_network.stop();
	m_status = FREE_STATUS;
	is_rev_board=false;
	m_human_black=false;

	if(redraw_){
		m_engine.init_snapshot(start_fen);
		redraw();
	}
}
void Board::rev_game()
{
	is_rev_board=1-is_rev_board;
	m_human_black=1-m_human_black;
	redraw();
}

void Board::start_robot()
{
	m_status = FIGHT_STATUS;

	m_robot.start();
	m_robot.send_ctrl_command("ucci\n");
	new_game(m_status);
}

void Board::set_level_config(int _depth,int _idle,int _style,int _knowledge,int _pruning,int _randomness,bool _usebook)
{
	m_search_depth = _depth;
	m_usebook = _usebook;

}

void Board::set_war_time(int _step_time,int _play_time)
{
	limit_count_time = _step_time;
	red_time = _play_time*60;
	black_time = _play_time* 60;

}

void Board::set_level()
{

	/** test simple*/
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
	//m_network.start();
	new_game(NETWORK_STATUS);
	//fd_send_skt = init_send_socket();

}

void Board::new_game(BOARD_STATUS _status)
{
	//m_status = FIGHT_STATUS;
	m_status = _status;

	m_engine.init_snapshot(start_fen);

	if(m_status == FIGHT_STATUS){
		m_robot.send_ctrl_command("setoption newgame\n");
		set_level();
	}


	moves_lines.clear();
	moves_lines = postion_str + std::string(start_fen);
	redraw();

	parent.textview_engine_log_clear();
	//parent.change_play(m_engine.red_player());
	parent.change_play(is_human_player());

	timer=Glib::signal_timeout().connect(sigc::mem_fun(*this,&Board::go_time),1000);
	/**如果是用户选择黑方，则电脑先走棋 -- if user choose black,the robot go moves first*/
	if(m_human_black){
		if(m_status == FIGHT_STATUS){
			moves_lines =moves_lines +std::string(" -- 0 1 ");
			m_robot.send_ctrl_command(moves_lines.c_str());
			m_robot.send_ctrl_command("\n");
			char str_cmd[256];
			sprintf(str_cmd,"go depth %d \n",m_search_depth);
			m_robot.send_ctrl_command(str_cmd);
		}else if(m_status == NETWORK_STATUS){

		}

	}

	parent.set_red_war_time(to_time_ustring(red_time),to_time_ustring(0));
	parent.set_black_war_time(to_time_ustring(black_time),to_time_ustring(0));
}

bool Board::network_log(const Glib::IOCondition& condition)
{
#if 0
	char buf[1024];
	int buf_len = 1024;
	char* p = buf;
	for (; buf_len > 0; ) {
		int len = m_network.get_network_log(p, buf_len);
		if (len <= 0)
			break;
		p += len;
		buf_len -= len;
	}

	if (buf_len > 0) {
		*p = 0;
		printf(buf);
		std::string str_buf(buf);

		size_t pos_=str_buf.find("draw");
		if(pos_ != std::string::npos){

			printf("计算机同意和棋\n");
			if (parent.on_end_game(ROBOT_DRAW)) {
				if(timer.connected())
					timer.disconnect();
				return 1;
			}
		}
		pos_=str_buf.find("resign");
		if(pos_ != std::string::npos){

			if(timer.connected())
				timer.disconnect();
			parent.on_end_game(ROBOT_LOSE);
			return true;
		}
		pos_=str_buf.find("nobestmove");
		if(pos_ != std::string::npos){
			if(timer.connected())
				timer.disconnect();
			parent.on_end_game(ROBOT_LOSE);
			return true;
		}
		size_t pos=str_buf.find("bestmoves:");
		if(pos != std::string::npos){
			std::string t_mv=str_buf.substr(pos+10,4);
			std::cout<<"get network mv = "<<t_mv<<std::endl;
			int mv = m_engine.iccs_str_to_move(t_mv);
			try_move(mv);
		}
	}

#endif
	return true;

}
bool Board::robot_log(const Glib::IOCondition& condition)
{
	/*for testing,delete me*/
	char buf[1024];
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
		printf(buf);
		std::string str_buf(buf);
		parent.show_textview_engine_log(str_buf);

		size_t pos_=str_buf.find("draw");
		if(pos_ != std::string::npos){

			printf("计算机同意和棋\n");
			if (parent.on_end_game(ROBOT_DRAW)) {
				if(timer.connected())
					timer.disconnect();
				return 1;
			}
		}
		pos_=str_buf.find("resign");
		if(pos_ != std::string::npos){

			if(timer.connected())
				timer.disconnect();
			parent.on_end_game(ROBOT_LOSE);
			return true;
		}
		pos_=str_buf.find("nobestmove");
		if(pos_ != std::string::npos){
			if(timer.connected())
				timer.disconnect();
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

Glib::ustring Board::to_time_ustring(int ival)
{
	char sp[32];
	sprintf(sp,"%02d:%02d", ival/60,ival%60);
	return Glib::ustring(sp);
}

Glib::ustring Board::to_msec_ustring(int ival)
{
	char sp[32];
	sprintf(sp,"%d000",ival);
	return Glib::ustring(sp);
}

bool Board::go_time()
{
	if(is_human_player()){
		count_time++;
		red_time--;
		parent.set_red_war_time(to_time_ustring(red_time),to_time_ustring(count_time));
		/** when the user 's step time over the limit time,we must warn it,and when over time the has not go,it will be losed */
		if(count_time>limit_count_time-10 && count_time<= limit_count_time){
			printf("time out,you less time: %d\n",limit_count_time-count_time);
			reckon_time_sound(limit_count_time-count_time);
		}
		else if(count_time > limit_count_time || red_time<0 ){
			printf(" time limit ,you lose\n");
			if(timer.connected())
				timer.disconnect();
			parent.on_end_game(HUMAN_OVER_TIME);
			count_time =0;
			if(is_network_game())
				send_to_socket("timeout");

		}
	}
	else{
		black_time--;
		count_time++;
		parent.set_black_war_time(to_time_ustring(black_time),to_time_ustring(count_time));
		//printf("black_time: %d\n",black_time);
		if(count_time>limit_count_time-10 && count_time<=limit_count_time){
			printf("time out,bot waster much time\n");
			m_robot.send_ctrl_command("stop\n");
		}
		else if(count_time >limit_count_time || black_time<0 ){
			printf(" time limit ,robot lose\n");
			if(timer.connected())
				timer.disconnect();
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
			this->set_size_request(521,577);
			break;
		case SMALL_BOARD:
			is_small_board=true;
			this->set_size_request(221,277);
			break;
		default:
			g_warn_if_reached();
			break;
	}
}

void Board::reckon_time_sound(int time_)
{
	switch(time_){
		case 0:
			CSound::play(SND_0);
			break;
		case 1:
			CSound::play(SND_1);
			break;
		case 2:
			CSound::play(SND_2);
			break;
		case 3:
			CSound::play(SND_3);
			break;
		case 4:
			CSound::play(SND_4);
			break;
		case 5:
			CSound::play(SND_5);
			break;
		case 6:
			CSound::play(SND_6);
			break;
		case 7:
			CSound::play(SND_7);
			break;
		case 8:
			CSound::play(SND_8);
			break;
		case 9:
			CSound::play(SND_9);
			break;
		case 10:
			CSound::play(SND_10);
			break;
		default:
			break;

	}

}

void Board::watch_socket(int fd)
{
	fd_recv_skt=fd;
	Glib::signal_io().connect(sigc::mem_fun(*this,&Board::on_network_io),
			fd_recv_skt, Glib::IO_IN);

}
bool Board::on_network_io(const Glib::IOCondition& )
{

	int fd_cli = -1;
       EC_THROW(-1 == (fd_cli = accept(fd_recv_skt, NULL, 0)));
       char buf[1024];
       size_t len = read(fd_cli, &buf[0], 1023);
       buf[len]=0;
       if (len > 0) {

		std::string str_buf(buf);
		size_t pos;
		//size_t pos_=str_buf.find("network-game-red");

		if((pos = str_buf.find("moves:")) != std::string::npos){
			std::string t_mv=str_buf.substr(pos+6,4);
			std::cout<<"get robot mv = "<<t_mv<<std::endl;
			int mv = m_engine.iccs_str_to_move(t_mv);
			try_move(mv);

		}else if((pos = str_buf.find("network-game-red,"))!= std::string::npos){
			//start network game with red player
			std::string enemy_name,my_name;
			size_t pos_s,pos_e,pos_m;
			pos_s= str_buf.find("enemy_name:");
			pos_e= str_buf.find(",my_name:");
			pos_m= str_buf.find_first_of("@");
			enemy_name = str_buf.substr(pos_s+11,pos_m-pos_s-11);
			pos_m = str_buf.find_last_of("@");
			my_name = str_buf.substr(pos_e+9,pos_m-pos_e-9);
			
			//parent.on_network_game(my_name,enemy_name,true);
			parent.on_network_game(enemy_name,my_name,true);
		}else if((pos = str_buf.find("network-game-black,")) != std::string::npos){
			//start network game with black player
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
			if(timer.connected())
				timer.disconnect();
			parent.on_end_game(ROBOT_LOSE);
			return true;
		}else if(str_buf.find("resign") != std::string::npos){
			if(timer.connected())
				timer.disconnect();
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
			if(timer.connected())
				timer.disconnect();
			parent.on_end_game(ROBOT_DRAW);
		}else if(str_buf.find("enemy_name:") != std::string::npos){

		}else if(str_buf.find("my_name:") !=std::string::npos){

		}





#if 0
		if(pos_ != std::string::npos){
			//start network game with red player
			parent.on_network_game("lerosua","enemy",true);
		}
		pos_=str_buf.find("network-game-black");
		if(pos_ != std::string::npos){
			//start network game with black player
			parent.on_network_game("lerosua","enemy",false);
		}
		pos_ = str_buf.find("network-game-win");
		if(pos_ !=std::string::npos){
			// i win the game.
			if(timer.connected())
				timer.disconnect();
			parent.on_end_game(ROBOT_LOSE);
			return true;
		}
		pos_ = str_buf.find("resign");
		if(pos_ != std::string::npos){

			if(timer.connected())
				timer.disconnect();
			parent.on_end_game(ROBOT_LOSE);
			return true;
		}
		size_t pos=str_buf.find("moves:");
		if(pos != std::string::npos){
			std::string t_mv=str_buf.substr(pos+6,4);
			std::cout<<"get robot mv = "<<t_mv<<std::endl;
			int mv = m_engine.iccs_str_to_move(t_mv);
			try_move(mv);
		}

		if(0 == strncmp("[{game:gmchess,",buf,15)){
			printf("get cmd %s\n",buf);
			string cmd=string(buf);
			network_package* net_pac = parse_network_package(cmd);

			if(net_pac->action=="ask"){
				//对方请求的包
				if(net_pac->status == "start"){
					//对方请求游戏

				}


			}else if(net_pac->action == "reply"){
				//对方回复的包


			}else if(net_pac->action == "working"){
				//对方走棋的包

				int mv = m_engine.iccs_str_to_move(net_pac->moves);
				try_move(mv);

			}



		}
#endif
	}
	close(fd_cli);
	return true;
}


int Board::init_send_socket()
{
	int sockfd;
	char buf[1024];
	struct sockaddr_in srvaddr;

	EC_THROW(-1 == (sockfd=socket(AF_INET,SOCK_STREAM,0)));
	bzero(&srvaddr,sizeof(srvaddr));
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
	char buf[1024];
	struct sockaddr_in srvaddr;

	EC_THROW(-1 == (sockfd=socket(AF_INET,SOCK_STREAM,0)));
	bzero(&srvaddr,sizeof(srvaddr));
	srvaddr.sin_family=AF_INET;
	srvaddr.sin_port=htons(GMPORT+1);
	srvaddr.sin_addr.s_addr=htonl(INADDR_ANY);

	int on = 1;
	EC_THROW( -1 == (setsockopt( sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) )));

	if( 0 == connect(sockfd,(struct sockaddr*)&srvaddr,sizeof(srvaddr))){
		write(sockfd,cmd_.c_str(),cmd_.size());
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
