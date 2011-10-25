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
//#include <libglademm/xml.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include "Board.h"
#define main_ui  DATA_DIR"/gmchess.glade"

class Board;
class BookView;
class ConfWindow;

class MainWindow:public Gtk::Window
{
	public:
		MainWindow();
		~MainWindow();
		/** 回到最初局面*/
		void on_first_move();
		/** 去到最后的局面*/
		void on_last_move();
		/** 执行下一步*/
		void on_next_move();
		/** 执行上一步*/
		void on_back_move();
		/** 开始AI对战*/
		void on_begin_game();
		/** start chanhue game*/
		void on_chanju_game();
		/** 开始网络对战*/
		/**
		 * @param me_ 已方名字
		 * @param enemy_ 对方名字
		 * @param role_red_ 已方角色
		 */
		void on_network_game(const std::string& me_,const std::string& enemy_,bool role_red_=true);
		/** gmchess最开始运行带的参数解释函数*/
		void start_with(const std::string& param);
		/** 认输*/
		void on_lose_game();
		/** 求和*/
		void on_draw_game();
		/** 悔棋*/
		void on_rue_game();
		/** 结束对局，跳出对话框*/
		bool on_end_game(OVERSTATUS _over);
		/** treeview 双击事件回调*/
		bool on_treeview_click(GdkEventButton* ev);
		void set_comment(const std::string& f_comment);
		/** 将引擎的返回显示到相关textview里 */
		void show_textview_engine_log(const std::string& f_text);
		/** 清空引擎textview里的内容*/
		void textview_engine_log_clear();
		/** 添加treeview数据*/
		void add_step_line(int num,const Glib::ustring& f_line);
		/** 删除treeview的最后一行,悔棋用的*/
		void del_step_last_line();
		/** 显示出当前步时位置*/
		void show_treeview_step();
		bool on_foreach_iter(const Gtk::TreeModel::iterator iter);
		void open_file(const std::string& filename);
		void change_play(bool player);
		void set_red_war_time(const Glib::ustring& f_time,const Glib::ustring& c_time);
		void set_black_war_time(const Glib::ustring& f_time,const Glib::ustring& c_time);
		void on_conf_window_close();
		void on_conf_window_quit();
		/** 根据大小调整棋盘*/
		/** resize the chess by board*/
		void on_size_change();
		void save_conf();
		void watch_socket(int fd);
		bool on_delete_event(GdkEventAny* event);
		void info_window(const std::string& info);
		/**  用于网络对战自动保存棋谱*/
		void auto_save_chess_file();
	protected:
		/** 初始化配置文件*/
		void init_conf();
		void init_ui_manager();
		void on_menu_open_file();
		void on_menu_save_file();
		void on_menu_save_board_to_png();
		void on_menu_file_quit();
		void on_menu_view_preferences();
		void on_menu_war_to_ai();
		void on_menu_free_play();
		void on_menu_rev_play();
		void on_menu_help();
		void on_menu_about();
		/** 初始化着法treeview */
		void init_move_treeview();
		/** 设置棋盘相关信息*/
		void set_information();
		/**各种模式转换 --- 设置着法按钮可见性*/
		void set_status();
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
		typedef Glib::RefPtr < Gtk::Builder> GBuilderXML;
		Board*				board;
		GBuilderXML			ui_xml;
		Glib::RefPtr<Gtk::UIManager>	ui_manager;
		Glib::RefPtr<Gtk::ActionGroup> 	action_group;
		Gtk::Widget*			menubar;
		Gtk::TreeView			m_treeview;
		BookView*			m_bookview;
		Gtk::ButtonBox*			buttonbox_war;
		Gtk::TextView*			text_comment;
		Gtk::TextView*			text_engine_log;
		Gtk::Notebook*			m_notebook;
		ConfWindow*			confwindow;
		Glib::RefPtr<Gdk::Pixbuf>	ui_logo ;
		Gtk::Image*			p1_image;
		Gtk::Image*			p2_image;
		Gtk::Label*			p1_step_time;
		Gtk::Label*			p2_step_time;
		Gtk::Label*			p1_war_time;
		Gtk::Label*			p2_war_time;
		Gtk::Label*			p1_name;
		Gtk::Label*			p2_name;

		/** 读谱时着法的按钮*/
		Gtk::Button*			btn_start;
		Gtk::Button*			btn_end ;
		Gtk::Button*			btn_prev;
		Gtk::Button*			btn_next;
		Gtk::Button*			btn_chanjue;
		/** 对战按钮*/
		Gtk::Button*			btn_begin;
		Gtk::Button*			btn_lose;
		Gtk::Button*			btn_draw;
		Gtk::Button*			btn_rue;

};


#endif   /* ----- #ifndef MAINWINDOW_FILE_HEADER_INC  ----- */
