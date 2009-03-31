/*
 * =====================================================================================
 * 
 *       Filename:  MainWindow.h
 * 
 *    Description:  主窗口
 * 
 *        Version:  1.0
 *        Created:  2009年02月14日 07时49分10秒 CST
 *       Revision:  none
 *       Compiler:  gcc
 * 
 *         Author:  lerosua (), lerosua@gmail.com
 *        Company:  Cyclone
 * 
 * =====================================================================================
 */

#ifndef  MAINWINDOW_FILE_HEADER_INC
#define  MAINWINDOW_FILE_HEADER_INC

#include <gtkmm.h>
#include <gtkmm/window.h>
#include <libglademm/xml.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include "Board.h"
#define main_ui  DATA_DIR"/gmchess.glade"
typedef Glib::RefPtr < Gnome::Glade::Xml > GlademmXML;

class Board;
class BookView;

class MainWindow:public Gtk::Window
{
	public:
		MainWindow();
		~MainWindow();
		/** 回到最初局面*/
		void on_start_move();
		/** 去到最后的局面*/
		void on_end_move();
		/** 执行下一步*/
		void on_next_move();
		/** 执行上一步*/
		void on_back_move();
		/** 开始对战*/
		void on_begin_game();
		/** 认输*/
		void on_lose_game();
		/** 求和*/
		void on_draw_game();
		/** 悔棋*/
		void on_rue_game();
		/** treeview 双击事件回调*/
		bool on_treeview_click(GdkEventButton* ev);
		void set_comment(const std::string& f_comment);
		/** 添加treeview数据*/
		void add_step_line(int num,const Glib::ustring& f_line);
		/** 删除treeview的最后一行,悔棋用的*/
		void del_step_last_line();
		/** 显示出当前步时位置*/
		void show_treeview_step();
		bool on_foreach_iter(const Gtk::TreeModel::iterator iter);
		void open_file(const std::string& filename);
	protected:
		void init_ui_manager();
		void on_menu_open_file();
		void on_menu_save_file();
		void on_menu_file_quit();
		void on_menu_view_preferences();
		void on_menu_war_to_ai();
		void on_menu_free_play();
		void on_menu_help();
		void on_menu_about();
		/** 初始化着法treeview */
		void init_move_treeview();
		/** 设置棋盘相关信息*/
		void set_information();
		/** 设置着法按钮可见性*/
		void set_move_button_property();
	public:
		class Columns:public Gtk::TreeModel::ColumnRecord
		{
			public:
				Columns()
				{
					add(step_num);
					add(step_bout);
					add(player);
					add(step_line);
				}
				Gtk::TreeModelColumn <int> step_num;
				/** 着法回合,红黑各一着为一回合*/
				Gtk::TreeModelColumn <int> step_bout;
				Gtk::TreeModelColumn <Glib::ustring> player;
				Gtk::TreeModelColumn <Glib::ustring> step_line;
		};
		Columns m_columns;
		Glib::RefPtr<Gtk::ListStore>	m_refTreeModel;
		
	private:
		Board*				board;
		GlademmXML			ui_xml;
		Glib::RefPtr<Gtk::UIManager>	ui_manager;
		Glib::RefPtr<Gtk::ActionGroup> 	action_group;
		Gtk::Widget*			menubar;
		Gtk::TreeView			m_treeview;
		BookView*			m_bookview;
		Gtk::ButtonBox*			buttonbox_war;
		Gtk::TextView*			text_comment;
		Glib::RefPtr<Gdk::Pixbuf>	ui_logo ;
		/** 读谱时着法的按钮*/
		Gtk::Button*			btn_start;
		Gtk::Button*			btn_end ;
		Gtk::Button*			btn_prev;
		Gtk::Button*			btn_next;
		/** 对战按钮*/
		Gtk::Button*			btn_begin;
		Gtk::Button*			btn_lose;
		Gtk::Button*			btn_draw;
		Gtk::Button*			btn_rue;
};


#endif   /* ----- #ifndef MAINWINDOW_FILE_HEADER_INC  ----- */
