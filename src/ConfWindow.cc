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
#include <glib/gi18n.h>

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

gboolean ConfWindow::delete_event_cb(GtkWindow*, gpointer data)
{
	return static_cast<ConfWindow*>(data)->on_delete_event();
}

ConfWindow::ConfWindow(GtkWindow* parent_window)
	: builder(NULL)
	, window(NULL)
	, cbtheme(NULL)
	, colorBt(NULL)
	, size_big_button(NULL)
	, usebook_button(NULL)
	, depth_button(NULL)
	, step_time_entry(NULL)
	, play_time_entry(NULL)
	, engine_name_entry(NULL)
	, m_usebook(false)
{
	GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
	gtk_widget_set_margin_top(vbox, 12);
	gtk_widget_set_margin_bottom(vbox, 12);
	gtk_widget_set_margin_start(vbox, 12);
	gtk_widget_set_margin_end(vbox, 12);

	GtkWidget* size_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
	GtkCheckButton* small_button = GTK_CHECK_BUTTON(gtk_check_button_new_with_label(_("Small board")));
	size_big_button = GTK_CHECK_BUTTON(gtk_check_button_new_with_label(_("Big board")));
	gtk_check_button_set_group(size_big_button, small_button);
	gtk_box_append(GTK_BOX(size_row), GTK_WIDGET(small_button));
	gtk_box_append(GTK_BOX(size_row), GTK_WIDGET(size_big_button));
	gtk_box_append(GTK_BOX(vbox), size_row);

	GtkWidget* theme_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
	gtk_box_append(GTK_BOX(theme_row), gtk_label_new(_("Theme:")));
	cbtheme = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());
	gtk_combo_box_text_append_text(cbtheme, "wood");
	gtk_combo_box_text_append_text(cbtheme, "west");
	gtk_box_append(GTK_BOX(theme_row), GTK_WIDGET(cbtheme));
	gtk_box_append(GTK_BOX(vbox), theme_row);

	GtkWidget* color_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
	gtk_box_append(GTK_BOX(color_row), gtk_label_new(_("Trace color:")));
	colorBt = GTK_COLOR_BUTTON(gtk_color_button_new());
	gtk_box_append(GTK_BOX(color_row), GTK_WIDGET(colorBt));
	gtk_box_append(GTK_BOX(vbox), color_row);

	GtkWidget* engine_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
	gtk_box_append(GTK_BOX(engine_row), gtk_label_new(_("Engine:")));
	engine_name_entry = GTK_ENTRY(gtk_entry_new());
	gtk_widget_set_hexpand(GTK_WIDGET(engine_name_entry), TRUE);
	gtk_box_append(GTK_BOX(engine_row), GTK_WIDGET(engine_name_entry));
	gtk_box_append(GTK_BOX(vbox), engine_row);

	usebook_button = GTK_CHECK_BUTTON(gtk_check_button_new_with_label(_("Use opening book")));
	gtk_box_append(GTK_BOX(vbox), GTK_WIDGET(usebook_button));

	GtkWidget* depth_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
	gtk_box_append(GTK_BOX(depth_row), gtk_label_new(_("Depth:")));
	depth_button = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(1.0, 15.0, 1.0));
	gtk_box_append(GTK_BOX(depth_row), GTK_WIDGET(depth_button));
	gtk_box_append(GTK_BOX(vbox), depth_row);

	GtkWidget* step_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
	gtk_box_append(GTK_BOX(step_row), gtk_label_new(_("Step time:")));
	step_time_entry = GTK_ENTRY(gtk_entry_new());
	gtk_box_append(GTK_BOX(step_row), GTK_WIDGET(step_time_entry));
	gtk_box_append(GTK_BOX(vbox), step_row);

	GtkWidget* play_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
	gtk_box_append(GTK_BOX(play_row), gtk_label_new(_("Play time:")));
	play_time_entry = GTK_ENTRY(gtk_entry_new());
	gtk_box_append(GTK_BOX(play_row), GTK_WIDGET(play_time_entry));
	gtk_box_append(GTK_BOX(vbox), play_row);

	GtkWidget* button_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
	gtk_widget_set_halign(button_row, GTK_ALIGN_END);
	GtkButton* cancel_bt = GTK_BUTTON(gtk_button_new_with_label(_("_Cancel")));
	GtkButton* ok_bt = GTK_BUTTON(gtk_button_new_with_label(_("_OK")));
	gtk_box_append(GTK_BOX(button_row), GTK_WIDGET(cancel_bt));
	gtk_box_append(GTK_BOX(button_row), GTK_WIDGET(ok_bt));
	gtk_box_append(GTK_BOX(vbox), button_row);

	GtkAdjustment* adjust = gtk_spin_button_get_adjustment(depth_button);
	gtk_adjustment_set_lower(adjust, 1.0);
	gtk_adjustment_set_upper(adjust, 15.0);
	gtk_adjustment_set_step_increment(adjust, 1.0);

	g_signal_connect(ok_bt, "clicked", G_CALLBACK(button_save_cb), this);
	g_signal_connect(cancel_bt, "clicked", G_CALLBACK(button_cancel_cb), this);
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

	gtk_combo_box_set_active(GTK_COMBO_BOX(cbtheme), m_theme == "wood" ? 0 : 1);
	gtk_check_button_set_active(size_big_button, m_size_big);
	gtk_check_button_set_active(usebook_button, m_usebook);
	gtk_spin_button_set_value(depth_button, atof(m_depth.c_str()));
	gtk_editable_set_text(GTK_EDITABLE(step_time_entry), m_step_time.c_str());
	gtk_editable_set_text(GTK_EDITABLE(play_time_entry), m_play_time.c_str());
	gtk_editable_set_text(GTK_EDITABLE(engine_name_entry), m_engine_name.c_str());

	window = gtk_window_new();
	gtk_window_set_child(GTK_WINDOW(window), vbox);
	gtk_window_set_title(GTK_WINDOW(window), "GMChess Preferences");
	if(parent_window)
		gtk_window_set_transient_for(GTK_WINDOW(window), parent_window);
	g_signal_connect(window, "close-request", G_CALLBACK(delete_event_cb), this);

	gtk_widget_show(window);
}

ConfWindow::~ConfWindow()
{
	if(window)
		gtk_window_destroy(GTK_WINDOW(window));
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
	m_usebook = gtk_check_button_get_active(usebook_button);
	m_size_big = gtk_check_button_get_active(size_big_button);
	m_depth = std::to_string(gtk_spin_button_get_value_as_int(depth_button));
	m_step_time = gtk_entry_buffer_get_text(gtk_entry_get_buffer(step_time_entry));
	m_play_time = gtk_entry_buffer_get_text(gtk_entry_get_buffer(play_time_entry));
	m_engine_name = gtk_entry_buffer_get_text(gtk_entry_get_buffer(engine_name_entry));

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
