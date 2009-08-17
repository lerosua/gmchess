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
#include "Engine.h"
#include "MainWindow.h"
#include "Sound.h"

/** 边界的宽度*/
/**  width of border */
const int border_width = 32;
/** 棋子的宽度*/
/** width of chessman */
const int chessman_width = 57;

#define	PLACE_LEFT 0x01
#define PLACE_RIGHT 0x02
#define PIECE_START 16  //棋子开始数字 start number of chessman
#define PIECE_END   48  //棋子结束数字 end number of chessman
#define PLACE_ALL PLACE_LEFT | PLACE_RIGHT
#define IMGAGE_DIR DATA_DIR"/wood/"
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
	selected_chessman(-1)
	,postion_str("position fen ")
	,parent(win)
	,red_time(500)
	,black_time(500)
	,count_time(0)
{

	std::list<Gtk::TargetEntry> listTargets;
	listTargets.push_back(Gtk::TargetEntry("STRING"));
	listTargets.push_back(Gtk::TargetEntry("text/plain"));

	this->set_size_request(521,577);

	this->drag_dest_set(listTargets);
	this->signal_drag_data_received().connect(
			sigc::mem_fun(*this, &Board::on_drog_data_received));

	/** 加载所需要图片进内存*/
	load_images();
	
	p_pgnfile=new Pgnfile(m_engine);
	m_engine.init_snapshot(start_fen);
	m_robot.set_out_slot(sigc::mem_fun(*this, &Board::robot_log));
	this->set_events(Gdk::BUTTON_PRESS_MASK|Gdk::EXPOSURE_MASK);
	this->show_all();



}

Board::~Board()
{
	if(timer.connected())
		timer.disconnect();
	m_robot.send_ctrl_command("quit\n");
	m_robot.stop();
}

void Board::load_images()
{
	bg_image = Gdk::Pixbuf::create_from_file(IMGAGE_DIR"bg.png");
	this->set_size_request(521,577);
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
	if(is_fight_to_robot()){
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
				selected_chessman = m_engine.get_piece(selected_x, selected_y);
				if(selected_chessman==0){
					/** 仍然没选中, still not select */
					selected_chessman = -1;
					printf("still no select chessman\n");
					return true;
				}
				/** 对战状态中，选了黑色棋子无效*/
				/** choose the black chessman is useless on war */
				if((is_fight_to_robot()) &&
					(selected_chessman>31)){
						printf("choose black %d\n",selected_chessman);
						selected_chessman =-1;
						//draw_select_frame(false); 
						return true;
				}

				draw_select_frame(true);
			}
		}
		else{
			/** 之前已经选中棋子，现在是生成着法或取消*/
			/** there has a selecter, now buider the moves or canel */
			int dst_chessman = m_engine.get_piece(selected_x,selected_y);
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
	int px = p.get_x() - 57 / 2;
	int py = p.get_y() - 57 / 2;

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
	m_engine.get_xy_from_chess(selected_chessman,sx,sy);
	Gdk::Point p = get_coordinate(sx, sy);


	int px = p.get_x() - 57 / 2;
	int py = p.get_y() - 57 / 2;


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
			draw_chessman(i, j, m_engine.get_piece(i, j));
		}
	}
}

void Board::start_move()
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

	redraw();
}

void Board::end_move()
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

	int dst = m_engine.get_dst_xy(dst_x,dst_y);
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

		/** 对战时的处理*/
		if(is_fight_to_robot()){
			if(eat){
				moves_lines.clear();
				moves_lines =postion_str+ m_engine.get_last_fen_from_snapshot()+std::string(" -- 0 1 ");
			}
			else{
				std::string iccs_str=m_engine.move_to_iccs_str(mv);
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
				Glib::ustring str_cmd="go time "+to_msec_ustring(black_time)+" increment 0\n";
				DLOG("send command to tell robot\n");
				DLOG("%s\n",str_cmd.c_str());
				m_robot.send_ctrl_command(str_cmd.c_str());
				//m_robot.send_ctrl_command("go time 295000 increment 0\n");
			}


			parent.change_play(is_human_player());
			count_time=0;
		}
		/**被将死了*/
		/** is it  mate */
		if(m_engine.mate()){
			parent.on_mate_game();
			DLOG("将军死棋\n");
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

}

void Board::rue_move()
{

	if(m_engine.how_step()<1)
		return;
	DLOG(" how_step %d\n",m_engine.how_step());
	int mv = m_engine.get_last_move_from_snapshot();
	DLOG(" undo move %d\n");
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

void Board::free_game()
{
	if(timer.connected())
		timer.disconnect();

	m_robot.send_ctrl_command("quit\n");
	m_robot.stop();
	m_status = FREE_STATUS;
	red_time=500;
	black_time=500;

	m_engine.init_snapshot(start_fen);
	redraw();
}

void Board::start_robot()
{
	m_status = FIGHT_STATUS;

	m_robot.start();
	m_robot.send_ctrl_command("ucci\n");
	new_game();
}

void Board::set_level()
{

	/** test simple*/
	m_robot.send_ctrl_command("setoption idle large\n");
	m_robot.send_ctrl_command("setoption style risky\n");
	m_robot.send_ctrl_command("setoption knowledge none\n");
	m_robot.send_ctrl_command("setoption pruning  large\n");
	m_robot.send_ctrl_command("setoption randomness large\n");
	//m_robot.send_ctrl_command("ucci\n");
}

void Board::new_game()
{
	m_status = FIGHT_STATUS;

	m_engine.init_snapshot(start_fen);
	set_level();
	m_robot.send_ctrl_command("setoption newgame\n");


	moves_lines.clear();
	moves_lines = postion_str + std::string(start_fen);
	redraw();

	parent.change_play(m_engine.red_player());

	timer=Glib::signal_timeout().connect(sigc::mem_fun(*this,&Board::go_time),1000);

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
		size_t pos_=str_buf.find("draw");
		if(pos_ != std::string::npos){

			printf("计算机同意和棋\n");

			return true;
		}
		pos_=str_buf.find("resign");
		if(pos_ != std::string::npos){

			parent.on_mate_game();
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
		//printf("red_time : %d\n",red_time);
	}
	else{
		black_time--;
		count_time++;
		parent.set_black_war_time(to_time_ustring(black_time),to_time_ustring(count_time));
		//printf("black_time: %d\n",black_time);
	}

	return true;
}
