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
#include "Board.h"
#define main_ui  "../data/gmchess.glade"
typedef Glib::RefPtr < Gnome::Glade::Xml > GlademmXML;

class Board;

class MainWindow:public Gtk::Window
{
	public:
		MainWindow();
		~MainWindow();
		void onNextMove();
		void onBackMove();
	protected:
		void init_ui_manager();
		void on_menu_open_file();
		void on_menu_file_quit();
		void on_menu_view_preferences();
		void on_menu_help_about();
		
	private:
		Board*				board;
		GlademmXML			ui_xml;
		Glib::RefPtr<Gtk::UIManager>	ui_manager;
		Glib::RefPtr<Gtk::ActionGroup> 	action_group;
		Gtk::Widget*			menubar;
};

#endif   /* ----- #ifndef MAINWINDOW_FILE_HEADER_INC  ----- */

