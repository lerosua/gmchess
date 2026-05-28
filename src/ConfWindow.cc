/*
 * =====================================================================================
 *
 *       Filename:  ConfWindow.cc
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2009年06月19日 21时41分51秒 CST
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  lerosua (), lerosua@gmail.com
 *        Company:  Cyclone
 *
 * =====================================================================================
 */

#include "ConfWindow.h"
#include "gmchess.h"

#include <cstdlib>

static GtkWidget* builder_widget(GtkBuilder* builder, const char* name)
{
	return GTK_WIDGET(gtk_builder_get_object(builder, name));
}

void ConfWindow::button_save_cb(GtkButton*, gpointer data)
{
	static_cast<ConfWindow*>(data)->on_button_save();
}

void ConfWindow::button_cancel_cb(GtkButton*, gpointer data)
{
	static_cast<ConfWindow*>(data)->on_button_cancel();
}

void ConfWindow::color_set_cb(GtkColorButton*, gpointer data)
{
	static_cast<ConfWindow*>(data)->on_button_color_set();
}

gboolean ConfWindow::delete_event_cb(GtkWidget*, GdkEvent*, gpointer data)
{
	return static_cast<ConfWindow*>(data)->on_delete_event();
}

ConfWindow::ConfWindow(GtkWindow* parent_window)
	: builder(NULL)
	, window(NULL)
	, cbtheme(NULL)
	, colorBt(NULL)
	, m_usebook(false)
{
	builder = gtk_builder_new_from_file(conf_ui);
	if(!builder)
		exit(271);

	GtkWidget* vbox = builder_widget(builder, "conf_vbox");

	GtkSpinButton* spinbt = GTK_SPIN_BUTTON(builder_widget(builder, "sb_depth"));
	GtkAdjustment* adjust = gtk_spin_button_get_adjustment(spinbt);
	gtk_adjustment_set_lower(adjust, 1.0);
	gtk_adjustment_set_upper(adjust, 15.0);
	gtk_adjustment_set_step_increment(adjust, 1.0);

	GtkButton* bt = GTK_BUTTON(builder_widget(builder, "button_ok"));
	g_signal_connect(bt, "clicked", G_CALLBACK(button_save_cb), this);
	bt = GTK_BUTTON(builder_widget(builder, "button_cancel"));
	g_signal_connect(bt, "clicked", G_CALLBACK(button_cancel_cb), this);

	colorBt = GTK_COLOR_BUTTON(builder_widget(builder, "colorbutton"));
	g_signal_connect(colorBt, "color-set", G_CALLBACK(color_set_cb), this);

	m_line_color = GMConf["line_color"];
	GdkRGBA color;
	if(gdk_rgba_parse(&color, m_line_color.c_str()))
		gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(colorBt), &color);

	std::string& size_big = GMConf["desktop_size"];
	m_size_big = (!size_big.empty())&&(size_big[0]=='1');
	std::string& use_book = GMConf["usebook"];
	m_usebook = (!use_book.empty()) && (use_book[0] == '1');
	m_depth = GMConf["engine_depth"] ;
	if(!GMConf["step_time"].empty())
		m_step_time = GMConf["step_time"] ;
	if(!GMConf["play_time"].empty())
		m_play_time = GMConf["play_time"] ;
	m_theme = GMConf["themes"];
	m_engine_name = GMConf["engine_name"];

	GtkBox* hbox = GTK_BOX(builder_widget(builder, "hbox_theme"));
	cbtheme = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());
	gtk_combo_box_text_append_text(cbtheme, "wood");
	gtk_combo_box_text_append_text(cbtheme, "west");
	gtk_combo_box_set_active(GTK_COMBO_BOX(cbtheme), m_theme == "wood" ? 0 : 1);
	gtk_box_pack_start(hbox, GTK_WIDGET(cbtheme), FALSE, FALSE, 0);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(builder_widget(builder, "b_radiobutton")), m_size_big);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(builder_widget(builder, "cb_book")), m_usebook);
	gtk_spin_button_set_value(spinbt, atof(m_depth.c_str()));
	gtk_entry_set_text(GTK_ENTRY(builder_widget(builder, "entry_step_time")), m_step_time.c_str());
	gtk_entry_set_text(GTK_ENTRY(builder_widget(builder, "entry_play_time")), m_play_time.c_str());
	gtk_entry_set_text(GTK_ENTRY(builder_widget(builder, "entry_engine_name")), m_engine_name.c_str());

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	gtk_window_set_title(GTK_WINDOW(window), "GMChess Preferences");
	if(parent_window)
		gtk_window_set_transient_for(GTK_WINDOW(window), parent_window);
	g_signal_connect(window, "delete-event", G_CALLBACK(delete_event_cb), this);

	gtk_widget_show_all(window);
}

ConfWindow::~ConfWindow()
{
	if(window)
		gtk_widget_destroy(window);
	if(builder)
		g_object_unref(builder);
}

void ConfWindow::raise()
{
	gtk_window_present(GTK_WINDOW(window));
}

void ConfWindow::on_button_save()
{
	write_to_GMConf();
	if(quit_callback)
		quit_callback();
	on_button_cancel();
}

void ConfWindow::on_button_cancel()
{
	if(close_callback)
		close_callback();
}

void ConfWindow::on_button_color_set()
{
	GdkRGBA color;
	gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(colorBt), &color);
	gchar* color_string = gdk_rgba_to_string(&color);
	m_line_color = color_string;
	g_free(color_string);
}

gboolean ConfWindow::on_delete_event()
{
	on_button_cancel();
	return TRUE;
}

void ConfWindow::write_to_GMConf()
{
	m_usebook = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(builder_widget(builder, "cb_book")));
	m_size_big = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(builder_widget(builder, "b_radiobutton")));
	m_depth = std::to_string(gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(builder_widget(builder, "sb_depth"))));
	m_step_time = gtk_entry_get_text(GTK_ENTRY(builder_widget(builder, "entry_step_time")));
	m_play_time = gtk_entry_get_text(GTK_ENTRY(builder_widget(builder, "entry_play_time")));
	m_engine_name = gtk_entry_get_text(GTK_ENTRY(builder_widget(builder, "entry_engine_name")));

	GMConf["usebook"] = m_usebook? "1":"0";
	GMConf["desktop_size"] = m_size_big?"1":"0";
	GMConf["engine_depth"] = m_depth;
	GMConf["step_time"] = m_step_time;
	GMConf["play_time"] = m_play_time;
	GMConf["engine_name"] = m_engine_name;
	GMConf["line_color"] = m_line_color;

	gchar* active_theme = gtk_combo_box_text_get_active_text(cbtheme);
	if(active_theme && std::string(active_theme) == "wood")
		GMConf["themes"] = "wood";
	else
		GMConf["themes"] = "west";
	g_free(active_theme);
}
