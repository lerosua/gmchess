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
#include "variablesmap.h"

#define conf_ui DATA_DIR"/confwin.glade"

class MainWindow;
class ConfWindow: public Gtk::Window
{

	public:
		ConfWindow(MainWindow* parent_);
		~ConfWindow();
		void on_button_save();
		void on_button_cancel();
		void on_button_color_set();
		bool on_delete_event(GdkEventAny*);
		typedef sigc::signal<void> type_signal_quit;
		type_signal_quit signal_quit()
		{ return signal_quit_; }
		void write_to_GMConf();
	private:
		typedef Glib::RefPtr < Gtk::Builder> GBuilderXML;
		GBuilderXML			vbox_xml;
		MainWindow* parent;
		type_signal_quit signal_quit_;
		VariablesMap* m_pVariablesMap;
		Gtk::ComboBoxText* cbtheme;
		Gtk::ColorButton* colorBt;
		Glib::ustring			m_depth;
		Glib::ustring			m_step_time;
		Glib::ustring			m_play_time;
		Glib::ustring			m_theme;
		Glib::ustring			m_engine_name;
		Glib::ustring			m_line_color;
		bool				m_size_big;
		bool				m_usebook;

};

#endif   /* ----- #ifndef CONFWINDOW_FILE_HEADER_INC  ----- */

