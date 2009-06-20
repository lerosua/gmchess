/*
 * =====================================================================================
 * 
 *       Filename:  ConfWindow.h
 * 
 *    Description:  config window class
 * 
 *        Version:  1.0
 *        Created:  2009年06月19日 21时40分28秒 CST
 *       Revision:  none
 *       Compiler:  gcc
 * 
 *         Author:  lerosua (), lerosua@gmail.com
 *        Company:  Cyclone
 * 
 * =====================================================================================
 */

#ifndef  CONFWINDOW_FILE_HEADER_INC
#define  CONFWINDOW_FILE_HEADER_INC
#include <gtkmm.h>
class MainWindow;
class ConfWindow: public Gtk::Window
{

	public:
		ConfWindow(MainWindow* parent_);
		~ConfWindow();
		void on_button_save();
		void on_button_cancel();
		bool on_delete_event(GdkEventAny*);
	private:
		MainWindow* parent;

};

#endif   /* ----- #ifndef CONFWINDOW_FILE_HEADER_INC  ----- */

