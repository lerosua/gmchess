/*
 * =====================================================================================
 *
 *       Filename:  MainWindow.h
 *
 *    Description:  主窗口
 *
 * =====================================================================================
 */

#ifndef  GMCHESS_MAINWINDOW_FILE_HEADER_INC
#define  GMCHESS_MAINWINDOW_FILE_HEADER_INC

#include <gtk/gtk.h>
#include <map>
#include <string>
#include "Board.h"

#define main_ui  DATA_DIR"/gmchess.glade"

class Board;
class BookView;
class ConfWindow;

class MainWindow
{
	public:
		MainWindow(GtkApplication* app = NULL);
		~MainWindow();
		GtkWidget* widget() { return window; }
		GtkWindow* gobj() { return GTK_WINDOW(window); }

		void on_first_move();
		void on_last_move();
		void on_next_move();
		void on_back_move();
		void on_begin_game();
		void on_chanju_game();
		void on_network_game(const std::string& me_,const std::string& enemy_,bool role_red_=true);
		void start_with(const std::string& param);
		void on_lose_game();
		void on_draw_game();
		void on_rue_game();
		bool on_end_game(OVERSTATUS _over);
		void set_comment(const std::string& f_comment);
		void show_textview_engine_log(const std::string& f_text);
		void textview_engine_log_clear();
		void add_step_line(int num,const std::string& f_line);
		void del_step_last_line();
		void show_treeview_step();
		void open_file(const std::string& filename);
		void change_play(bool player);
		void set_red_war_time(const std::string& f_time,const std::string& c_time);
		void set_black_war_time(const std::string& f_time,const std::string& c_time);
		void on_conf_window_close();
		void on_conf_window_quit();
		void on_size_change();
		void save_conf();
		void watch_socket(int fd);
		gboolean on_delete_event();
		void info_window(const std::string& info);
		void auto_save_chess_file();

	protected:
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
		void init_move_treeview();
		void set_information();
		void set_status();

	private:
		static void button_first_cb(GtkButton* button, gpointer data);
		static void button_last_cb(GtkButton* button, gpointer data);
		static void button_back_cb(GtkButton* button, gpointer data);
		static void button_next_cb(GtkButton* button, gpointer data);
		static void button_begin_cb(GtkButton* button, gpointer data);
		static void button_lose_cb(GtkButton* button, gpointer data);
		static void button_draw_cb(GtkButton* button, gpointer data);
		static void button_rue_cb(GtkButton* button, gpointer data);
		static void button_chanju_cb(GtkButton* button, gpointer data);
		static void move_list_activate_cb(GtkListView* list, guint position, gpointer data);
		static gboolean delete_event_cb(GtkWindow* window, gpointer data);
		static void window_destroy_cb(GtkWidget* widget, gpointer data);
		static void board_resize_cb(GtkDrawingArea* area, int width, int height, gpointer data);
		static void menu_open_cb(GSimpleAction* action, GVariant* parameter, gpointer data);
		static void menu_save_cb(GSimpleAction* action, GVariant* parameter, gpointer data);
		static void menu_save_board_cb(GSimpleAction* action, GVariant* parameter, gpointer data);
		static void menu_quit_cb(GSimpleAction* action, GVariant* parameter, gpointer data);
		static void menu_preferences_cb(GSimpleAction* action, GVariant* parameter, gpointer data);
		static void menu_war_ai_cb(GSimpleAction* action, GVariant* parameter, gpointer data);
		static void menu_free_play_cb(GSimpleAction* action, GVariant* parameter, gpointer data);
		static void menu_rev_play_cb(GSimpleAction* action, GVariant* parameter, gpointer data);
		static void menu_help_cb(GSimpleAction* action, GVariant* parameter, gpointer data);
		static void menu_about_cb(GSimpleAction* action, GVariant* parameter, gpointer data);

		GtkWidget* builder_widget(const char* name);
		GtkWidget* remember_widget(const char* name, GtkWidget* widget);
		void build_main_ui(GtkApplication* app);

		Board*				board;
		GtkBuilder*			ui_xml;
		GtkWidget*			window;
		GtkWidget*			menubar;
		GSimpleActionGroup*	action_group;
		std::map<std::string, GtkWidget*> ui_widgets;
		GtkListView*		m_treeview;
		GListStore*			m_refTreeModel;
		GtkSingleSelection*	m_move_selection;
		BookView*			m_bookview;
		GtkWidget*			buttonbox_war;
		GtkTextView*		text_comment;
		GtkTextView*		text_engine_log;
		GtkNotebook*		m_notebook;
		ConfWindow*			confwindow;
		GdkPaintable*		ui_logo ;
		GtkImage*			p1_image;
		GtkImage*			p2_image;
		GtkLabel*			p1_step_time;
		GtkLabel*			p2_step_time;
		GtkLabel*			p1_war_time;
		GtkLabel*			p2_war_time;
		GtkLabel*			p1_name;
		GtkLabel*			p2_name;

		GtkButton*			btn_start;
		GtkButton*			btn_end ;
		GtkButton*			btn_prev;
		GtkButton*			btn_next;
		GtkButton*			btn_chanjue;
		GtkButton*			btn_begin;
		GtkButton*			btn_lose;
		GtkButton*			btn_draw;
		GtkButton*			btn_rue;
};

#endif
