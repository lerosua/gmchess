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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <functional>
#include <gtk/gtk.h>
#include <string>

#define conf_ui DATA_DIR"/confwin.glade"

class ConfWindow
{

	public:
		typedef std::function<void()> Callback;

		ConfWindow(GtkWindow* parent_window);
		~ConfWindow();
		void on_button_save();
		void on_button_cancel();
		void on_button_color_set();
		gboolean on_delete_event();
		void raise();
		void set_quit_callback(const Callback& callback)
		{ quit_callback = callback; }
		void set_close_callback(const Callback& callback)
		{ close_callback = callback; }
		void write_to_GMConf();
	private:
		static void button_save_cb(GtkButton* button, gpointer data);
		static void button_cancel_cb(GtkButton* button, gpointer data);
		static void color_set_cb(GtkColorButton* button, gpointer data);
		static gboolean delete_event_cb(GtkWindow* window, gpointer data);

		GtkBuilder*		builder;
		GtkWidget*		window;
		GtkComboBoxText*	cbtheme;
		GtkColorButton*	colorBt;
		GtkCheckButton*	size_big_button;
		GtkCheckButton*	usebook_button;
		GtkSpinButton*	depth_button;
		GtkEntry*		step_time_entry;
		GtkEntry*		play_time_entry;
		GtkEntry*		engine_name_entry;
		Callback		quit_callback;
		Callback		close_callback;
		std::string		m_depth = "10";
		std::string		m_step_time = "240";
		std::string		m_play_time = "40";
		std::string		m_theme;
		std::string		m_engine_name;
		std::string		m_line_color;
		bool				m_size_big = true;
		bool				m_usebook;

};

#endif   /* ----- #ifndef CONFWINDOW_FILE_HEADER_INC  ----- */
