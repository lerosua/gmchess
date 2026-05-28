/*
 * =====================================================================================
 *
 *       Filename:  MainWindow.cc
 *
 *    Description:
 *
 * =====================================================================================
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "MainWindow.h"
#include "BookView.h"
#include "ConfWindow.h"
#include "Sound.h"
#include "gmchess.h"
#include "paths.h"

#include <glib/gi18n.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>

enum {
	COL_STEP_NUM,
	COL_STEP_BOUT,
	COL_PLAYER,
	COL_STEP_LINE,
	N_STEP_COLUMNS
};

static GObject* move_row_new(int num, const std::string& line)
{
	const int bout = (num + 1) / 2;
	const char* player = (num % 2) == 0 ? _("Black") : _("Red");
	char display[1024];
	snprintf(display, sizeof(display), "%d    %s    %s", bout, player, line.c_str());

	GObject* row = G_OBJECT(g_object_new(G_TYPE_OBJECT, NULL));
	g_object_set_data(row, "step-num", GINT_TO_POINTER(num));
	g_object_set_data(row, "step-bout", GINT_TO_POINTER(bout));
	g_object_set_data_full(row, "player", g_strdup(player), g_free);
	g_object_set_data_full(row, "step-line", g_strdup(line.c_str()), g_free);
	g_object_set_data_full(row, "display", g_strdup(display), g_free);
	return row;
}

static int row_int(GObject* row, const char* key)
{
	return GPOINTER_TO_INT(g_object_get_data(row, key));
}

static const char* row_string(GObject* row, const char* key)
{
	const char* value = static_cast<const char*>(g_object_get_data(row, key));
	return value ? value : "";
}

static void move_list_setup_cb(GtkSignalListItemFactory*, GtkListItem* item, gpointer)
{
	GtkWidget* label = gtk_label_new("");
	gtk_label_set_xalign(GTK_LABEL(label), 0.0);
	gtk_widget_set_margin_start(label, 6);
	gtk_widget_set_margin_end(label, 6);
	gtk_list_item_set_child(item, label);
}

static void move_list_bind_cb(GtkSignalListItemFactory*, GtkListItem* item, gpointer)
{
	GObject* row = G_OBJECT(gtk_list_item_get_item(item));
	GtkWidget* label = gtk_list_item_get_child(item);
	gtk_label_set_text(GTK_LABEL(label), row_string(row, "display"));
}

struct DialogRunData {
	GMainLoop* loop;
	gint response;
	GFile* file;
	bool save;
};

static void alert_dialog_done_cb(GObject* source, GAsyncResult* result, gpointer data)
{
	DialogRunData* run_data = static_cast<DialogRunData*>(data);
	GError* error = NULL;
	int button = gtk_alert_dialog_choose_finish(GTK_ALERT_DIALOG(source), result, &error);
	if(error) {
		g_error_free(error);
		run_data->response = GTK_RESPONSE_CANCEL;
	}
	else {
		run_data->response = (button == 0) ? GTK_RESPONSE_CANCEL : GTK_RESPONSE_OK;
	}
	g_main_loop_quit(run_data->loop);
}

static void file_dialog_done_cb(GObject* source, GAsyncResult* result, gpointer data)
{
	DialogRunData* run_data = static_cast<DialogRunData*>(data);
	GError* error = NULL;
	GtkFileDialog* dialog = GTK_FILE_DIALOG(source);
	run_data->file = run_data->save
		? gtk_file_dialog_save_finish(dialog, result, &error)
		: gtk_file_dialog_open_finish(dialog, result, &error);
	if(error) {
		g_error_free(error);
		run_data->response = GTK_RESPONSE_CANCEL;
	}
	else {
		run_data->response = run_data->file ? GTK_RESPONSE_ACCEPT : GTK_RESPONSE_CANCEL;
	}
	g_main_loop_quit(run_data->loop);
}

static gint run_message_dialog(GtkWindow* parent, const char* title,
		GtkMessageType type, GtkButtonsType buttons, const std::string& secondary)
{
	(void)type;
	GtkAlertDialog* dialog = gtk_alert_dialog_new("%s", title);
	gtk_alert_dialog_set_modal(dialog, TRUE);
	if(!secondary.empty())
		gtk_alert_dialog_set_detail(dialog, secondary.c_str());

	const char* ok_buttons[] = { _("_Cancel"), _("_OK"), NULL };
	const char* info_buttons[] = { _("_OK"), NULL };
	if(buttons == GTK_BUTTONS_OK_CANCEL) {
		gtk_alert_dialog_set_buttons(dialog, ok_buttons);
		gtk_alert_dialog_set_cancel_button(dialog, 0);
		gtk_alert_dialog_set_default_button(dialog, 1);
	}
	else {
		gtk_alert_dialog_set_buttons(dialog, info_buttons);
		gtk_alert_dialog_set_cancel_button(dialog, 0);
		gtk_alert_dialog_set_default_button(dialog, 0);
	}

	DialogRunData run_data = { g_main_loop_new(NULL, FALSE), GTK_RESPONSE_NONE, NULL, false };
	gtk_alert_dialog_choose(dialog, parent, NULL, alert_dialog_done_cb, &run_data);
	g_main_loop_run(run_data.loop);
	g_main_loop_unref(run_data.loop);
	g_object_unref(dialog);
	if(buttons != GTK_BUTTONS_OK_CANCEL && run_data.response == GTK_RESPONSE_CANCEL)
		return GTK_RESPONSE_OK;
	return run_data.response;
}

static GtkFileFilter* create_file_filter(const char* name, const char* lower, const char* upper)
{
	GtkFileFilter* filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, name);
	gtk_file_filter_add_pattern(filter, lower);
	gtk_file_filter_add_pattern(filter, upper);
	return filter;
}

static std::string run_file_chooser(GtkWindow* parent, const char* title,
		GtkFileChooserAction action, const char* accept_label, bool add_filters)
{
	GtkFileDialog* dialog = gtk_file_dialog_new();
	gtk_file_dialog_set_title(dialog, title);
	gtk_file_dialog_set_accept_label(dialog, accept_label);
	gtk_file_dialog_set_modal(dialog, TRUE);

	GListStore* filters = NULL;
	if(add_filters) {
		filters = g_list_store_new(GTK_TYPE_FILE_FILTER);
		const char* specs[][3] = {
			{ "PGN", "*.pgn", "*.PGN" },
			{ "CCM", "*.ccm", "*.CCM" },
			{ "CHE", "*.che", "*.CHE" },
			{ "CHN", "*.chn", "*.CHN" },
			{ "MXQ", "*.mxq", "*.MXQ" },
			{ "XQF", "*.xqf", "*.XQF" },
		};
		for(size_t i = 0; i < G_N_ELEMENTS(specs); ++i) {
			GtkFileFilter* filter = create_file_filter(specs[i][0], specs[i][1], specs[i][2]);
			g_list_store_append(filters, filter);
			g_object_unref(filter);
		}
		GtkFileFilter* filter_any = create_file_filter(_("All Files"), "*", "*");
		g_list_store_append(filters, filter_any);
		g_object_unref(filter_any);
		gtk_file_dialog_set_filters(dialog, G_LIST_MODEL(filters));
	}

	DialogRunData run_data = {
		g_main_loop_new(NULL, FALSE),
		GTK_RESPONSE_NONE,
		NULL,
		action == GTK_FILE_CHOOSER_ACTION_SAVE
	};
	if(run_data.save)
		gtk_file_dialog_save(dialog, parent, NULL, file_dialog_done_cb, &run_data);
	else
		gtk_file_dialog_open(dialog, parent, NULL, file_dialog_done_cb, &run_data);
	g_main_loop_run(run_data.loop);
	g_main_loop_unref(run_data.loop);

	std::string filename;
	if(run_data.response == GTK_RESPONSE_ACCEPT && run_data.file) {
		char* path = g_file_get_path(run_data.file);
		if(path) {
			filename = path;
			g_free(path);
		}
		g_object_unref(run_data.file);
	}
	if(filters)
		g_object_unref(filters);
	g_object_unref(dialog);
	return filename;
}

MainWindow::MainWindow(GtkApplication* app)
	: board(NULL)
	, ui_xml(NULL)
	, window(NULL)
	, menubar(NULL)
	, action_group(NULL)
	, m_treeview(NULL)
	, m_refTreeModel(NULL)
	, m_move_selection(NULL)
	, m_bookview(NULL)
	, buttonbox_war(NULL)
	, text_comment(NULL)
	, text_engine_log(NULL)
	, m_notebook(NULL)
	, confwindow(NULL)
	, ui_logo(NULL)
{
	build_main_ui(app);
	g_signal_connect(window, "close-request", G_CALLBACK(delete_event_cb), this);
	g_signal_connect(window, "destroy", G_CALLBACK(window_destroy_cb), this);

	GtkWidget* box_board = builder_widget("vbox_board");
	buttonbox_war = builder_widget("hbuttonbox_war");
	text_comment = GTK_TEXT_VIEW(builder_widget("textview_comment"));
	text_engine_log = GTK_TEXT_VIEW(builder_widget("textview_engine_log"));
	m_notebook = GTK_NOTEBOOK(builder_widget("notebook"));
	btn_start = GTK_BUTTON(builder_widget("button_start"));
	btn_end = GTK_BUTTON(builder_widget("button_end"));
	btn_prev = GTK_BUTTON(builder_widget("button_prev"));
	btn_next = GTK_BUTTON(builder_widget("button_next"));
	btn_chanjue = GTK_BUTTON(builder_widget("button_chanju"));
	btn_begin = GTK_BUTTON(builder_widget("button_begin"));
	btn_lose = GTK_BUTTON(builder_widget("button_lose"));
	btn_draw = GTK_BUTTON(builder_widget("button_draw"));
	btn_rue = GTK_BUTTON(builder_widget("button_rue"));

	g_signal_connect(btn_start, "clicked", G_CALLBACK(button_first_cb), this);
	g_signal_connect(btn_end, "clicked", G_CALLBACK(button_last_cb), this);
	g_signal_connect(btn_prev, "clicked", G_CALLBACK(button_back_cb), this);
	g_signal_connect(btn_next, "clicked", G_CALLBACK(button_next_cb), this);
	g_signal_connect(btn_begin, "clicked", G_CALLBACK(button_begin_cb), this);
	g_signal_connect(btn_lose, "clicked", G_CALLBACK(button_lose_cb), this);
	g_signal_connect(btn_draw, "clicked", G_CALLBACK(button_draw_cb), this);
	g_signal_connect(btn_rue, "clicked", G_CALLBACK(button_rue_cb), this);
	g_signal_connect(btn_chanjue, "clicked", G_CALLBACK(button_chanju_cb), this);

	board = new Board(*this);
	gtk_box_append(GTK_BOX(box_board), board->widget());
	gtk_window_set_title(GTK_WINDOW(window), _("GMChess"));

	GError* error = NULL;
	ui_logo = GDK_PAINTABLE(gdk_texture_new_from_filename(
				gmchess_data_path("gmchess.png").c_str(), &error));
	if(error)
		g_error_free(error);

	init_ui_manager();
	GtkWidget* menu_tool_box = builder_widget("box_menu_toolbar");
	gtk_box_append(GTK_BOX(menu_tool_box), menubar);

	init_move_treeview();

	GtkWidget* scroll_book = builder_widget("scrolledwin_book");
	m_bookview = new BookView(this);
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll_book), m_bookview->widget());

	p1_image = GTK_IMAGE(builder_widget("image_p1"));
	p2_image = GTK_IMAGE(builder_widget("image_p2"));
	p1_war_time = GTK_LABEL(builder_widget("P1_war_time"));
	p2_war_time = GTK_LABEL(builder_widget("P2_war_time"));
	p1_step_time = GTK_LABEL(builder_widget("P1_step_time"));
	p2_step_time = GTK_LABEL(builder_widget("P2_step_time"));
	p1_name = GTK_LABEL(builder_widget("P1_name"));
	p2_name = GTK_LABEL(builder_widget("P2_name"));

	init_conf();
	g_signal_connect(board->widget(), "resize", G_CALLBACK(board_resize_cb), this);

	gtk_window_present(GTK_WINDOW(window));
	gtk_widget_set_visible(GTK_WIDGET(p1_image), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(p2_image), FALSE);

	if(atoi(GMConf["desktop_size"].c_str()) == 1)
		board->set_board_size(BIG_BOARD);
	else
		board->set_board_size(SMALL_BOARD);

	int depth = atoi(GMConf["engine_depth"].c_str());
	g_log("Mainwindow", G_LOG_LEVEL_INFO, "depth %d", depth);
	board->set_level_config(depth, 0, 0, 0, 0, 0, atoi(GMConf["usebook"].c_str()));

	std::string tmp = GMConf["step_time"];
	if(tmp.empty())
		tmp = "240";
	int step_time = atoi(tmp.c_str());
	tmp = GMConf["play_time"];
	if(tmp.empty())
		tmp = "60";
	int play_time = atoi(tmp.c_str());
	if(step_time > 0 && step_time < 600 && play_time > 0)
		board->set_time(step_time, play_time);

	std::string theme = GMConf["themes"];
	std::string engine_name = GMConf["engine_name"];
	std::string color = GMConf["line_color"];
	if(engine_name.empty())
		engine_name = "eleeye_engine";
	if(theme.empty())
		theme = "wood";
	board->set_themes(theme);
	board->set_engine(engine_name);
	board->set_trace_color(color);
}

MainWindow::~MainWindow()
{
	if(window)
		gtk_window_destroy(GTK_WINDOW(window));
	delete confwindow;
	delete m_bookview;
	delete board;
	if(m_refTreeModel)
		g_object_unref(m_refTreeModel);
	if(m_move_selection)
		g_object_unref(m_move_selection);
	if(ui_logo)
		g_object_unref(ui_logo);
	if(action_group)
		g_object_unref(action_group);
	if(ui_xml)
		g_object_unref(ui_xml);
}

GtkWidget* MainWindow::remember_widget(const char* name, GtkWidget* widget)
{
	ui_widgets[name] = widget;
	return widget;
}

GtkWidget* MainWindow::builder_widget(const char* name)
{
	std::map<std::string, GtkWidget*>::iterator found = ui_widgets.find(name);
	if(found == ui_widgets.end())
		g_error("Unable to find widget '%s' in %s", name, main_ui);
	return found->second;
}

void MainWindow::build_main_ui(GtkApplication* app)
{
	window = gtk_window_new();
	if(app)
		gtk_window_set_application(GTK_WINDOW(window), app);
	gtk_window_set_default_size(GTK_WINDOW(window), 900, 620);

	GtkWidget* main_box = remember_widget("main_window", gtk_box_new(GTK_ORIENTATION_VERTICAL, 4));
	gtk_window_set_child(GTK_WINDOW(window), main_box);

	GtkWidget* menu_box = remember_widget("box_menu_toolbar", gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
	gtk_box_append(GTK_BOX(main_box), menu_box);

	GtkWidget* content = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
	gtk_widget_set_vexpand(content, TRUE);
	gtk_box_append(GTK_BOX(main_box), content);

	GtkWidget* players = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
	gtk_widget_set_size_request(players, 140, -1);
	gtk_box_append(GTK_BOX(content), players);

	GtkWidget* p1_panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
	gtk_widget_set_vexpand(p1_panel, TRUE);
	gtk_box_append(GTK_BOX(players), p1_panel);

	p1_image = GTK_IMAGE(remember_widget("image_p1",
				gtk_image_new_from_file(gmchess_data_path("play.png").c_str())));
	p2_image = GTK_IMAGE(remember_widget("image_p2",
				gtk_image_new_from_file(gmchess_data_path("play.png").c_str())));
	gtk_image_set_pixel_size(p1_image, 36);
	gtk_image_set_pixel_size(p2_image, 36);
	GtkWidget* p1_avatar = gtk_picture_new_for_filename(gmchess_data_path("p1.png").c_str());
	gtk_picture_set_content_fit(GTK_PICTURE(p1_avatar), GTK_CONTENT_FIT_CONTAIN);
	gtk_widget_set_size_request(p1_avatar, 88, 88);
	gtk_widget_set_halign(p1_avatar, GTK_ALIGN_CENTER);
	gtk_widget_set_margin_top(p1_avatar, 18);
	gtk_widget_set_margin_bottom(p1_avatar, 4);
	gtk_box_append(GTK_BOX(p1_panel), p1_avatar);
	p1_name = GTK_LABEL(remember_widget("P1_name", gtk_label_new("Computer")));
	p1_step_time = GTK_LABEL(remember_widget("P1_step_time", gtk_label_new("")));
	p1_war_time = GTK_LABEL(remember_widget("P1_war_time", gtk_label_new("")));
	gtk_label_set_xalign(p1_name, 0.5);
	gtk_box_append(GTK_BOX(p1_panel), GTK_WIDGET(p1_name));
	gtk_box_append(GTK_BOX(p1_panel), GTK_WIDGET(p1_step_time));
	gtk_box_append(GTK_BOX(p1_panel), GTK_WIDGET(p1_war_time));
	GtkWidget* p1_spacer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_widget_set_vexpand(p1_spacer, TRUE);
	gtk_box_append(GTK_BOX(p1_panel), p1_spacer);
	gtk_widget_set_valign(GTK_WIDGET(p1_image), GTK_ALIGN_END);
	gtk_widget_set_halign(GTK_WIDGET(p1_image), GTK_ALIGN_CENTER);
	gtk_widget_set_margin_bottom(GTK_WIDGET(p1_image), 16);
	gtk_box_append(GTK_BOX(p1_panel), GTK_WIDGET(p1_image));
	gtk_box_append(GTK_BOX(players), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));

	GtkWidget* p2_panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
	gtk_widget_set_vexpand(p2_panel, TRUE);
	gtk_box_append(GTK_BOX(players), p2_panel);

	GtkWidget* p2_avatar = gtk_picture_new_for_filename(gmchess_data_path("p2.png").c_str());
	gtk_picture_set_content_fit(GTK_PICTURE(p2_avatar), GTK_CONTENT_FIT_CONTAIN);
	gtk_widget_set_size_request(p2_avatar, 88, 88);
	gtk_widget_set_halign(p2_avatar, GTK_ALIGN_CENTER);
	gtk_widget_set_margin_top(p2_avatar, 18);
	gtk_widget_set_margin_bottom(p2_avatar, 4);
	gtk_box_append(GTK_BOX(p2_panel), p2_avatar);
	p2_name = GTK_LABEL(remember_widget("P2_name", gtk_label_new("Human")));
	p2_step_time = GTK_LABEL(remember_widget("P2_step_time", gtk_label_new("")));
	p2_war_time = GTK_LABEL(remember_widget("P2_war_time", gtk_label_new("")));
	gtk_label_set_xalign(p2_name, 0.5);
	gtk_box_append(GTK_BOX(p2_panel), GTK_WIDGET(p2_name));
	gtk_box_append(GTK_BOX(p2_panel), GTK_WIDGET(p2_step_time));
	gtk_box_append(GTK_BOX(p2_panel), GTK_WIDGET(p2_war_time));
	GtkWidget* p2_spacer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_widget_set_vexpand(p2_spacer, TRUE);
	gtk_box_append(GTK_BOX(p2_panel), p2_spacer);
	gtk_widget_set_valign(GTK_WIDGET(p2_image), GTK_ALIGN_END);
	gtk_widget_set_halign(GTK_WIDGET(p2_image), GTK_ALIGN_CENTER);
	gtk_widget_set_margin_bottom(GTK_WIDGET(p2_image), 16);
	gtk_box_append(GTK_BOX(p2_panel), GTK_WIDGET(p2_image));

	GtkWidget* board_box = remember_widget("vbox_board", gtk_box_new(GTK_ORIENTATION_VERTICAL, 4));
	gtk_widget_set_hexpand(board_box, TRUE);
	gtk_widget_set_vexpand(board_box, TRUE);
	gtk_box_append(GTK_BOX(content), board_box);

	buttonbox_war = remember_widget("hbuttonbox_war", gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4));
	btn_begin = GTK_BUTTON(remember_widget("button_begin", gtk_button_new_with_mnemonic(_("Start"))));
	btn_lose = GTK_BUTTON(remember_widget("button_lose", gtk_button_new_with_mnemonic(_("Lost"))));
	btn_draw = GTK_BUTTON(remember_widget("button_draw", gtk_button_new_with_mnemonic(_("Draw"))));
	btn_rue = GTK_BUTTON(remember_widget("button_rue", gtk_button_new_with_mnemonic(_("Rue"))));
	gtk_box_append(GTK_BOX(buttonbox_war), GTK_WIDGET(btn_begin));
	gtk_box_append(GTK_BOX(buttonbox_war), GTK_WIDGET(btn_lose));
	gtk_box_append(GTK_BOX(buttonbox_war), GTK_WIDGET(btn_draw));
	gtk_box_append(GTK_BOX(buttonbox_war), GTK_WIDGET(btn_rue));
	gtk_box_append(GTK_BOX(board_box), buttonbox_war);

	m_notebook = GTK_NOTEBOOK(remember_widget("notebook", gtk_notebook_new()));
	gtk_widget_set_hexpand(GTK_WIDGET(m_notebook), TRUE);
	gtk_widget_set_vexpand(GTK_WIDGET(m_notebook), TRUE);
	gtk_box_append(GTK_BOX(content), GTK_WIDGET(m_notebook));

	GtkWidget* info_page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
	GtkWidget* info_label = remember_widget("info_label", gtk_label_new(_("Information")));
	gtk_label_set_wrap(GTK_LABEL(info_label), TRUE);
	gtk_box_append(GTK_BOX(info_page), info_label);
	GtkWidget* scrolled = remember_widget("scrolledwindow", gtk_scrolled_window_new());
	gtk_widget_set_vexpand(scrolled, TRUE);
	gtk_box_append(GTK_BOX(info_page), scrolled);

	GtkWidget* move_buttons = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
	btn_chanjue = GTK_BUTTON(remember_widget("button_chanju", gtk_button_new_from_icon_name("go-jump")));
	btn_start = GTK_BUTTON(remember_widget("button_start", gtk_button_new_from_icon_name("go-first")));
	btn_prev = GTK_BUTTON(remember_widget("button_prev", gtk_button_new_from_icon_name("go-previous")));
	btn_next = GTK_BUTTON(remember_widget("button_next", gtk_button_new_from_icon_name("go-next")));
	btn_end = GTK_BUTTON(remember_widget("button_end", gtk_button_new_from_icon_name("go-last")));
	gtk_box_append(GTK_BOX(move_buttons), GTK_WIDGET(btn_chanjue));
	gtk_box_append(GTK_BOX(move_buttons), GTK_WIDGET(btn_start));
	gtk_box_append(GTK_BOX(move_buttons), GTK_WIDGET(btn_prev));
	gtk_box_append(GTK_BOX(move_buttons), GTK_WIDGET(btn_next));
	gtk_box_append(GTK_BOX(move_buttons), GTK_WIDGET(btn_end));
	gtk_box_append(GTK_BOX(info_page), move_buttons);

	text_comment = GTK_TEXT_VIEW(remember_widget("textview_comment", gtk_text_view_new()));
	GtkWidget* comment_scroll = gtk_scrolled_window_new();
	gtk_widget_set_vexpand(comment_scroll, TRUE);
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(comment_scroll), GTK_WIDGET(text_comment));
	gtk_box_append(GTK_BOX(info_page), comment_scroll);
	gtk_notebook_append_page(m_notebook, info_page, gtk_label_new(_("Board Information")));

	GtkWidget* book_page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
	GtkWidget* book_scroll = remember_widget("scrolledwin_book", gtk_scrolled_window_new());
	gtk_widget_set_vexpand(book_scroll, TRUE);
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(book_scroll), NULL);
	gtk_box_append(GTK_BOX(book_page), book_scroll);
	gtk_notebook_append_page(m_notebook, book_page, gtk_label_new(_("Book")));

	GtkWidget* engine_page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
	text_engine_log = GTK_TEXT_VIEW(remember_widget("textview_engine_log", gtk_text_view_new()));
	GtkWidget* engine_scroll = gtk_scrolled_window_new();
	gtk_widget_set_vexpand(engine_scroll, TRUE);
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(engine_scroll), GTK_WIDGET(text_engine_log));
	gtk_box_append(GTK_BOX(engine_page), engine_scroll);
	gtk_notebook_append_page(m_notebook, engine_page, gtk_label_new(_("Engine")));
}

void MainWindow::button_first_cb(GtkButton*, gpointer data)
{
	static_cast<MainWindow*>(data)->on_first_move();
}

void MainWindow::button_last_cb(GtkButton*, gpointer data)
{
	static_cast<MainWindow*>(data)->on_last_move();
}

void MainWindow::button_back_cb(GtkButton*, gpointer data)
{
	static_cast<MainWindow*>(data)->on_back_move();
}

void MainWindow::button_next_cb(GtkButton*, gpointer data)
{
	static_cast<MainWindow*>(data)->on_next_move();
}

void MainWindow::button_begin_cb(GtkButton*, gpointer data)
{
	static_cast<MainWindow*>(data)->on_begin_game();
}

void MainWindow::button_lose_cb(GtkButton*, gpointer data)
{
	static_cast<MainWindow*>(data)->on_lose_game();
}

void MainWindow::button_draw_cb(GtkButton*, gpointer data)
{
	static_cast<MainWindow*>(data)->on_draw_game();
}

void MainWindow::button_rue_cb(GtkButton*, gpointer data)
{
	static_cast<MainWindow*>(data)->on_rue_game();
}

void MainWindow::button_chanju_cb(GtkButton*, gpointer data)
{
	static_cast<MainWindow*>(data)->on_chanju_game();
}

void MainWindow::move_list_activate_cb(GtkListView*, guint position, gpointer data)
{
	MainWindow* self = static_cast<MainWindow*>(data);
	if(self->board->is_fight_to_robot())
		return;
	GObject* row = G_OBJECT(g_list_model_get_item(G_LIST_MODEL(self->m_refTreeModel), position));
	if(!row)
		return;
	self->board->get_board_by_move(row_int(row, "step-num"));
	g_object_unref(row);
}

gboolean MainWindow::delete_event_cb(GtkWindow*, gpointer data)
{
	return static_cast<MainWindow*>(data)->on_delete_event();
}

void MainWindow::window_destroy_cb(GtkWidget*, gpointer data)
{
	static_cast<MainWindow*>(data)->window = NULL;
}

void MainWindow::board_resize_cb(GtkDrawingArea*, int, int, gpointer data)
{
	static_cast<MainWindow*>(data)->on_size_change();
}

void MainWindow::menu_open_cb(GSimpleAction*, GVariant*, gpointer data)
{
	static_cast<MainWindow*>(data)->on_menu_open_file();
}

void MainWindow::menu_save_cb(GSimpleAction*, GVariant*, gpointer data)
{
	static_cast<MainWindow*>(data)->on_menu_save_file();
}

void MainWindow::menu_save_board_cb(GSimpleAction*, GVariant*, gpointer data)
{
	static_cast<MainWindow*>(data)->on_menu_save_board_to_png();
}

void MainWindow::menu_quit_cb(GSimpleAction*, GVariant*, gpointer data)
{
	static_cast<MainWindow*>(data)->on_menu_file_quit();
}

void MainWindow::menu_preferences_cb(GSimpleAction*, GVariant*, gpointer data)
{
	static_cast<MainWindow*>(data)->on_menu_view_preferences();
}

void MainWindow::menu_war_ai_cb(GSimpleAction*, GVariant*, gpointer data)
{
	static_cast<MainWindow*>(data)->on_menu_war_to_ai();
}

void MainWindow::menu_free_play_cb(GSimpleAction*, GVariant*, gpointer data)
{
	static_cast<MainWindow*>(data)->on_menu_free_play();
}

void MainWindow::menu_rev_play_cb(GSimpleAction*, GVariant*, gpointer data)
{
	static_cast<MainWindow*>(data)->on_menu_rev_play();
}

void MainWindow::menu_help_cb(GSimpleAction*, GVariant*, gpointer data)
{
	static_cast<MainWindow*>(data)->on_menu_help();
}

void MainWindow::menu_about_cb(GSimpleAction*, GVariant*, gpointer data)
{
	static_cast<MainWindow*>(data)->on_menu_about();
}

void MainWindow::on_size_change()
{
	board->configure_board(gtk_widget_get_width(board->widget()));
}

void MainWindow::change_play(bool player)
{
	if(player) {
		gtk_widget_set_visible(GTK_WIDGET(p1_image), FALSE);
		gtk_widget_set_visible(GTK_WIDGET(p2_image), TRUE);
	}
	else {
		gtk_widget_set_visible(GTK_WIDGET(p2_image), FALSE);
		gtk_widget_set_visible(GTK_WIDGET(p1_image), TRUE);
	}
}

void MainWindow::save_conf()
{
	char buf[512];
	const char* homedir = g_get_user_config_dir();
	snprintf(buf, sizeof(buf), "%s/gmchess/config", homedir);
	std::ofstream file(buf);
	for(std::map<std::string, std::string>::iterator iter = GMConf.begin();
			iter != GMConf.end(); ++iter) {
		file << iter->first << "\t=\t" << iter->second << std::endl;
	}
	file.close();
}

void MainWindow::init_conf()
{
	char buf[512];
	char book_dir[512];
	char file_dir[512];
	const char* homedir = g_get_user_config_dir();

	snprintf(book_dir, sizeof(book_dir), "%s/gmchess/book", homedir);
	m_bookview->load_book_dir(book_dir);
	snprintf(buf, sizeof(buf), "%s/gmchess/config", homedir);

	std::ifstream file(buf);
	if(!file) {
#ifdef __APPLE__
		mkdir(homedir, S_IRUSR | S_IWUSR | S_IXUSR);
#endif
		char homepath[512];
		snprintf(homepath, sizeof(homepath), "%s/gmchess/", homedir);
		mkdir(homepath, S_IRUSR | S_IWUSR | S_IXUSR);
		GMConf["usebook"] = "1";
		GMConf["desktop_size"] = "1";
		GMConf["engine_depth"] = "5";
		GMConf["themes"] = "wood";
		GMConf["engine_name"] = "eleeye_engine";
		GMConf["line_color"] = "#198964";
		save_conf();

		snprintf(file_dir, sizeof(file_dir), "%s/gmchess/files", homedir);
		mkdir(file_dir, S_IRUSR | S_IWUSR | S_IXUSR);
		return;
	}

	std::string line;
	std::string name;
	std::string key;
	while(std::getline(file, line)) {
		size_t pos = line.find_first_of("=");
		if(pos == std::string::npos)
			continue;
		name = line.substr(0, pos);
		key = line.substr(pos + 1, std::string::npos);

		size_t pos1 = name.find_first_not_of(" \t");
		size_t pos2 = name.find_last_not_of(" \t");
		if(pos1 == std::string::npos || pos2 == std::string::npos)
			continue;
		name = name.substr(pos1, pos2 - pos1 + 1);

		pos1 = key.find_first_not_of(" \t");
		pos2 = key.find_last_not_of(" \t");
		if(pos1 == std::string::npos || pos2 == std::string::npos)
			continue;
		key = key.substr(pos1, pos2 - pos1 + 1);

		GMConf.insert(std::pair<std::string, std::string>(name, key));
	}
	file.close();
}

void MainWindow::set_comment(const std::string& f_comment)
{
	gtk_text_view_set_wrap_mode(text_comment, GTK_WRAP_WORD);
	GtkTextBuffer* buffer = gtk_text_view_get_buffer(text_comment);
	gtk_text_buffer_set_text(buffer, f_comment.c_str(), -1);
}

void MainWindow::show_textview_engine_log(const std::string& f_text)
{
	gtk_text_view_set_wrap_mode(text_engine_log, GTK_WRAP_WORD);
	GtkTextBuffer* buffer = gtk_text_view_get_buffer(text_engine_log);
	GtkTextIter end;
	gtk_text_buffer_get_end_iter(buffer, &end);
	gtk_text_buffer_insert(buffer, &end, f_text.c_str(), -1);
	gtk_text_buffer_get_end_iter(buffer, &end);
	gtk_text_buffer_place_cursor(buffer, &end);
	GtkTextMark* mark = gtk_text_buffer_get_insert(buffer);
	gtk_text_view_scroll_mark_onscreen(text_engine_log, mark);
}

void MainWindow::textview_engine_log_clear()
{
	GtkTextBuffer* buffer = gtk_text_view_get_buffer(text_engine_log);
	gtk_text_buffer_set_text(buffer, "", -1);
}

void MainWindow::show_treeview_step()
{
	if(!m_refTreeModel || !m_move_selection)
		return;

	int current_step = board->get_step();
	const guint count = g_list_model_get_n_items(G_LIST_MODEL(m_refTreeModel));
	for(guint i = 0; i < count; ++i) {
		GObject* row = G_OBJECT(g_list_model_get_item(G_LIST_MODEL(m_refTreeModel), i));
		if(row && row_int(row, "step-num") == current_step) {
			gtk_single_selection_set_selected(m_move_selection, i);
			g_object_unref(row);
			return;
		}
		if(row)
			g_object_unref(row);
	}
}

void MainWindow::on_next_move()
{
	board->next_move();
	show_treeview_step();
}

void MainWindow::on_back_move()
{
	board->back_move();
	show_treeview_step();
}

void MainWindow::on_first_move()
{
	board->first_move();
	if(m_move_selection)
		gtk_single_selection_set_selected(m_move_selection, 0);
}

void MainWindow::on_last_move()
{
	board->last_move();
	show_treeview_step();
}

void MainWindow::init_ui_manager()
{
	static const GActionEntry entries[] = {
		{ "open", menu_open_cb, NULL, NULL, NULL },
		{ "save", menu_save_cb, NULL, NULL, NULL },
		{ "save-board", menu_save_board_cb, NULL, NULL, NULL },
		{ "quit", menu_quit_cb, NULL, NULL, NULL },
		{ "preferences", menu_preferences_cb, NULL, NULL, NULL },
		{ "fight-ai", menu_war_ai_cb, NULL, NULL, NULL },
		{ "free-play", menu_free_play_cb, NULL, NULL, NULL },
		{ "switch-colour", menu_rev_play_cb, NULL, NULL, NULL },
		{ "help", menu_help_cb, NULL, NULL, NULL },
		{ "about", menu_about_cb, NULL, NULL, NULL },
	};

	action_group = g_simple_action_group_new();
	g_action_map_add_action_entries(G_ACTION_MAP(action_group), entries, G_N_ELEMENTS(entries), this);
	gtk_widget_insert_action_group(window, "win", G_ACTION_GROUP(action_group));

	GMenu* root = g_menu_new();
	GMenu* file_menu = g_menu_new();
	g_menu_append(file_menu, _("Open file"), "win.open");
	g_menu_append(file_menu, _("Save as"), "win.save");
	g_menu_append(file_menu, _("Save Board"), "win.save-board");
	g_menu_append(file_menu, _("_Quit"), "win.quit");
	g_menu_append_submenu(root, _("_File"), G_MENU_MODEL(file_menu));
	g_object_unref(file_menu);

	GMenu* view_menu = g_menu_new();
	g_menu_append(view_menu, _("_Switch colour"), "win.switch-colour");
	g_menu_append(view_menu, _("_Preferences"), "win.preferences");
	g_menu_append_submenu(root, _("_View"), G_MENU_MODEL(view_menu));
	g_object_unref(view_menu);

	GMenu* game_menu = g_menu_new();
	g_menu_append(game_menu, _("_Fight to AI"), "win.fight-ai");
	g_menu_append(game_menu, _("Free Play"), "win.free-play");
	g_menu_append_submenu(root, _("_Game"), G_MENU_MODEL(game_menu));
	g_object_unref(game_menu);

	GMenu* help_menu = g_menu_new();
	g_menu_append(help_menu, _("_Help"), "win.help");
	g_menu_append(help_menu, _("_About"), "win.about");
	g_menu_append_submenu(root, _("_Help"), G_MENU_MODEL(help_menu));
	g_object_unref(help_menu);

	menubar = gtk_popover_menu_bar_new_from_model(G_MENU_MODEL(root));
	g_object_unref(root);
}

void MainWindow::on_menu_save_board_to_png()
{
	board->save_board_to_file("/tmp/1.png");
}

void MainWindow::on_menu_save_file()
{
	std::string filename = run_file_chooser(gobj(), _("Save File"),
			GTK_FILE_CHOOSER_ACTION_SAVE, _("_Save"), false);
	if(filename.empty())
		return;

	std::ofstream file(filename.c_str());
	if(!file) {
		DLOG("open %s file error\n", filename.c_str());
		return;
	}

	const guint count = g_list_model_get_n_items(G_LIST_MODEL(m_refTreeModel));
	for(guint i = 0; i < count; ++i) {
		GObject* row = G_OBJECT(g_list_model_get_item(G_LIST_MODEL(m_refTreeModel), i));
		if(!row)
			continue;
		file << row_int(row, "step-bout") << ". " << row_string(row, "step-line");
		g_object_unref(row);

		if(i + 1 < count) {
			row = G_OBJECT(g_list_model_get_item(G_LIST_MODEL(m_refTreeModel), ++i));
			if(row) {
				file << "  " << row_string(row, "step-line") << std::endl;
				g_object_unref(row);
			}
		}
		else {
			file << std::endl;
		}
	}
	file.close();
}

void MainWindow::auto_save_chess_file()
{
	std::string homedir = g_get_user_config_dir();
	homedir += "/gmchess/files/";

	char time1[200];
	char time2[200];
	time_t now = time(NULL);
	struct tm* tmp = localtime(&now);
	strftime(time1, sizeof(time1), "%Y-%m-%d-%H-%M-", tmp);
	strftime(time2, sizeof(time2), "%Y.%m.%d", tmp);

	const char* p1 = gtk_label_get_text(p1_name);
	const char* p2 = gtk_label_get_text(p2_name);
	std::string name = std::string(p1) + "-" + p2 + ".pgn";
	std::string filename = homedir + std::string(time1) + name;

	std::ofstream file(filename.c_str());
	if(!file) {
		DLOG("open %s file error\n", filename.c_str());
		return;
	}

	file << "[Game \"Chese chess Play by GMChess\"]" << std::endl;
	file << "[Date \"" << std::string(time2) << "\"]" << std::endl;
	file << "[Red \"" << p1 << "\"]" << std::endl;
	file << "[Black \"" << p2 << "\"]" << std::endl;

	const guint count = g_list_model_get_n_items(G_LIST_MODEL(m_refTreeModel));
	for(guint i = 0; i < count; ++i) {
		GObject* row = G_OBJECT(g_list_model_get_item(G_LIST_MODEL(m_refTreeModel), i));
		if(!row)
			continue;
		file << row_int(row, "step-bout") << ". " << row_string(row, "step-line");
		g_object_unref(row);

		if(i + 1 < count) {
			row = G_OBJECT(g_list_model_get_item(G_LIST_MODEL(m_refTreeModel), ++i));
			if(row) {
				file << "  " << row_string(row, "step-line") << std::endl;
				g_object_unref(row);
			}
		}
		else {
			file << std::endl;
		}
	}
	file.close();
}

void MainWindow::on_menu_open_file()
{
	std::string filename = run_file_chooser(gobj(), _("Choose File"),
			GTK_FILE_CHOOSER_ACTION_OPEN, _("_Open"), true);
	if(!filename.empty())
		open_file(filename);
}

void MainWindow::open_file(const std::string& filename)
{
	if(board->is_fight_to_robot()) {
		gint result = run_message_dialog(gobj(), _("AI Warn"), GTK_MESSAGE_QUESTION,
				GTK_BUTTONS_OK_CANCEL,
				_("Open book view will close the AI game. Are you sure?"));
		if(result == GTK_RESPONSE_OK)
			board->free_game();
		else
			return;
	}

	int out;
	std::string::size_type const pos = filename.find(".pgn");
	if(pos == std::string::npos) {
		std::string const convert_cmdline = "convert_pgn \"" + filename + "\"";
		if(system(convert_cmdline.c_str()) < 0) {
			DLOG("convert pgn file error\n");
			return;
		}
		out = board->open_file("/tmp/gmchess.pgn");
	}
	else {
		out = board->open_file(filename);
	}

	if(out < 0) {
		DLOG("open file: %s error\n", filename.c_str());
		run_message_dialog(gobj(), "Error", GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
				_("the file maybe not right format for chess"));
	}
	else {
		init_move_treeview();
		set_information();
		gtk_notebook_set_current_page(m_notebook, 0);
	}
}

void MainWindow::info_window(const std::string& info)
{
	run_message_dialog(gobj(), "Info", GTK_MESSAGE_INFO, GTK_BUTTONS_OK, info);
}

void MainWindow::on_menu_war_to_ai()
{
	on_begin_game();
}

void MainWindow::on_menu_free_play()
{
	board->free_game();
	g_list_store_remove_all(m_refTreeModel);
	set_status();
}

void MainWindow::on_menu_rev_play()
{
	board->rev_game();
}

void MainWindow::on_menu_file_quit()
{
	if(board->get_status() == NETWORK_STATUS)
		board->send_to_socket("close");
	if(window)
		gtk_window_destroy(GTK_WINDOW(window));
}

gboolean MainWindow::on_delete_event()
{
	if(board->get_status() == NETWORK_STATUS) {
		gint result = run_message_dialog(gobj(), _("Warning"), GTK_MESSAGE_QUESTION,
				GTK_BUTTONS_OK_CANCEL,
				_("Are you  sure to close the network game?"));
		if(result == GTK_RESPONSE_OK)
			board->send_to_socket("close");
		else
			return TRUE;
	}
	return FALSE;
}

void MainWindow::on_menu_view_preferences()
{
	if(confwindow == NULL) {
		confwindow = new ConfWindow(gobj());
		confwindow->set_quit_callback([this]() {
				on_conf_window_quit();
				});
		confwindow->set_close_callback([this]() {
				on_conf_window_close();
				});
		confwindow->raise();
	}
	else {
		confwindow->raise();
	}
}

void MainWindow::on_conf_window_quit()
{
	save_conf();

	if(atoi(GMConf["desktop_size"].c_str()) == 1)
		board->set_board_size(BIG_BOARD);
	else
		board->set_board_size(SMALL_BOARD);
	board->set_themes(GMConf["themes"]);
}

void MainWindow::on_conf_window_close()
{
	if(confwindow != NULL) {
		delete confwindow;
		confwindow = NULL;
	}
}

void MainWindow::on_menu_help()
{
	run_message_dialog(gobj(), _("Function not implemented yet"), GTK_MESSAGE_INFO,
			GTK_BUTTONS_OK, "");
}

void MainWindow::on_menu_about()
{
	const gchar* authors[] = {
		"lerosua@gmail.com ",
		"wind(xihels@gmail.com)",
		NULL
	};
	GtkWidget* about = gtk_about_dialog_new();
	if(ui_logo)
		gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(about), ui_logo);
	gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(about), "GMChess");
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(about), PACKAGE_VERSION);
	gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(about), "https://lerosua.github.io");
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(about),
			"Copyright (c) 2009 - 2011 lerosua");
	gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(about),
			_("GMChess is a Chinese chess game written with GTK."));
	gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(about), authors);
	gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(about),
			_("This program is licensed under GNU General Public Licence (GPL) version 2."));
	gtk_about_dialog_set_translator_credits(GTK_ABOUT_DIALOG(about),
			"zh_CN lerosua@gmail.com\nru Sadovnikov Dmitry <xbadcode@mail.ru>");
	gtk_window_set_transient_for(GTK_WINDOW(about), gobj());
	gtk_window_present(GTK_WINDOW(about));
}

void MainWindow::add_step_line(int num, const std::string& f_line)
{
	GObject* row = move_row_new(num, f_line);
	g_list_store_append(m_refTreeModel, row);
	g_object_unref(row);
	if(m_move_selection)
		gtk_single_selection_set_selected(m_move_selection,
				g_list_model_get_n_items(G_LIST_MODEL(m_refTreeModel)) - 1);
}

void MainWindow::del_step_last_line()
{
	const guint count = g_list_model_get_n_items(G_LIST_MODEL(m_refTreeModel));
	if(count == 0)
		return;
	g_list_store_remove(m_refTreeModel, count - 1);
}

void MainWindow::init_move_treeview()
{
	if(m_treeview == NULL) {
		GtkWidget* scrolwin = builder_widget("scrolledwindow");
		GtkListItemFactory* factory = gtk_signal_list_item_factory_new();
		g_signal_connect(factory, "setup", G_CALLBACK(move_list_setup_cb), this);
		g_signal_connect(factory, "bind", G_CALLBACK(move_list_bind_cb), this);
		m_refTreeModel = g_list_store_new(G_TYPE_OBJECT);
		m_move_selection = gtk_single_selection_new(G_LIST_MODEL(g_object_ref(m_refTreeModel)));
		m_treeview = GTK_LIST_VIEW(gtk_list_view_new(GTK_SELECTION_MODEL(g_object_ref(m_move_selection)), factory));
		gtk_list_view_set_show_separators(m_treeview, TRUE);
		g_signal_connect(m_treeview, "activate", G_CALLBACK(move_list_activate_cb), this);
		gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolwin), GTK_WIDGET(m_treeview));
		gtk_widget_set_visible(GTK_WIDGET(m_treeview), TRUE);
		set_status();
		return;
	}

	g_list_store_remove_all(m_refTreeModel);
	const std::vector<std::string>& move_chinese = board->get_move_chinese_snapshot();
	for(std::vector<std::string>::const_iterator iter = move_chinese.begin();
			iter != move_chinese.end(); ++iter) {
		add_step_line((int)(iter - move_chinese.begin()) + 1, *iter);
	}
}

void MainWindow::set_information()
{
	GtkLabel* info = GTK_LABEL(builder_widget("info_label"));

	const Board_info& board_info = board->get_board_info();
	gtk_label_set_text(p1_name, board_info.black.c_str());
	gtk_label_set_text(p2_name, board_info.red.c_str());
	std::string text = std::string(_("Game: ")) + board_info.event + "\n";
	text += std::string(_("Time: ")) + board_info.date + "\n";
	text += std::string(_("Site: ")) + board_info.site + "\n";
	text += std::string(_("Result: ")) + board_info.result + "\n";
	text += std::string(_("Opening: ")) + board_info.opening + "\n";
	text += std::string(_("Variation:  ")) + board_info.variation + "\n";
	gtk_label_set_text(info, text.c_str());
	gtk_label_set_ellipsize(info, PANGO_ELLIPSIZE_END);
	set_status();
}

void MainWindow::set_status()
{
	int f_status = board->get_status();
	bool f_use = true;

	switch(f_status) {
		case READ_STATUS:
			gtk_widget_set_sensitive(GTK_WIDGET(btn_next), f_use);
			gtk_widget_set_sensitive(GTK_WIDGET(btn_prev), f_use);
			gtk_widget_set_sensitive(GTK_WIDGET(btn_start), f_use);
			gtk_widget_set_sensitive(GTK_WIDGET(btn_end), f_use);
			gtk_widget_set_sensitive(GTK_WIDGET(btn_begin), f_use);
			gtk_widget_set_sensitive(GTK_WIDGET(btn_lose), !f_use);
			gtk_widget_set_sensitive(GTK_WIDGET(btn_draw), !f_use);
			gtk_widget_set_sensitive(GTK_WIDGET(btn_rue), !f_use);
			break;
		case FIGHT_STATUS:
		case NETWORK_STATUS:
			gtk_widget_set_sensitive(GTK_WIDGET(btn_next), !f_use);
			gtk_widget_set_sensitive(GTK_WIDGET(btn_prev), !f_use);
			gtk_widget_set_sensitive(GTK_WIDGET(btn_start), !f_use);
			gtk_widget_set_sensitive(GTK_WIDGET(btn_end), !f_use);
			gtk_widget_set_sensitive(GTK_WIDGET(btn_begin), !f_use);
			gtk_widget_set_sensitive(GTK_WIDGET(btn_lose), f_use);
			gtk_widget_set_sensitive(GTK_WIDGET(btn_draw), f_use);
			gtk_widget_set_sensitive(GTK_WIDGET(btn_rue), f_use);
			break;
		case FREE_STATUS:
			gtk_widget_set_sensitive(GTK_WIDGET(btn_next), f_use);
			gtk_widget_set_sensitive(GTK_WIDGET(btn_prev), f_use);
			gtk_widget_set_sensitive(GTK_WIDGET(btn_start), f_use);
			gtk_widget_set_sensitive(GTK_WIDGET(btn_end), f_use);
			gtk_widget_set_sensitive(GTK_WIDGET(btn_begin), f_use);
			gtk_widget_set_sensitive(GTK_WIDGET(btn_lose), !f_use);
			gtk_widget_set_sensitive(GTK_WIDGET(btn_draw), !f_use);
			gtk_widget_set_sensitive(GTK_WIDGET(btn_rue), !f_use);
			break;
		default:
			break;
	};
}

void MainWindow::on_network_game(const std::string& me_name, const std::string& enemy_name,
		bool role_red_)
{
	gtk_label_set_text(p1_name, enemy_name.c_str());
	gtk_label_set_text(p2_name, me_name.c_str());
	if(!role_red_)
		board->rev_game();

	g_list_store_remove_all(m_refTreeModel);
	board->start_network();
	set_status();
	gtk_widget_set_sensitive(GTK_WIDGET(btn_begin), FALSE);
}

void MainWindow::on_chanju_game()
{
	if(board->is_fight_to_robot()) {
		gint result = run_message_dialog(gobj(), _("new game"), GTK_MESSAGE_QUESTION,
				GTK_BUTTONS_OK_CANCEL, _("Will you start a new game?"));
		if(result == GTK_RESPONSE_OK) {
			g_list_store_remove_all(m_refTreeModel);
			board->chanju_game();
		}
		return;
	}
	else if(board->is_network_game()) {
		run_message_dialog(gobj(), _("Information"), GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
				_("You are play online, Please end this game first!"));
		return;
	}
	g_list_store_remove_all(m_refTreeModel);
	board->start_robot(false);
	set_status();
	gtk_widget_set_sensitive(GTK_WIDGET(btn_begin), FALSE);
}

void MainWindow::on_begin_game()
{
	if(board->is_fight_to_robot()) {
		gint result = run_message_dialog(gobj(), _("new game"), GTK_MESSAGE_QUESTION,
				GTK_BUTTONS_OK_CANCEL, _("Will you start a new game?"));
		if(result == GTK_RESPONSE_OK) {
			g_list_store_remove_all(m_refTreeModel);
			board->new_game();
		}
		return;
	}
	else if(board->is_network_game()) {
		run_message_dialog(gobj(), _("Information"), GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
				_("You are playing online, Please end this game first!"));
		return;
	}
	g_list_store_remove_all(m_refTreeModel);
	board->start_robot();
	set_status();
	gtk_widget_set_sensitive(GTK_WIDGET(btn_begin), FALSE);
}

void MainWindow::on_lose_game()
{
	if(board->is_fight_to_robot()) {
		gint result = run_message_dialog(gobj(), _("be lose"), GTK_MESSAGE_QUESTION,
				GTK_BUTTONS_OK_CANCEL, _("Will you resign in this game?"));
		if(result == GTK_RESPONSE_OK) {
			board->free_game();
			set_status();
		}
		return;
	}
	else if(board->is_network_game()) {
		gint result = run_message_dialog(gobj(), _("be lose"), GTK_MESSAGE_QUESTION,
				GTK_BUTTONS_OK_CANCEL, _("Will you resign in this game?"));
		if(result == GTK_RESPONSE_OK) {
			board->send_to_socket("resign");
			run_message_dialog(gobj(), _("Information"), GTK_MESSAGE_INFO,
					GTK_BUTTONS_OK, _("You lose the game!"));
			board->free_game();
			set_status();
		}
	}
}

void MainWindow::on_draw_game()
{
	if(board->is_fight_to_robot()) {
		board->draw_move();
	}
	else if(board->is_network_game()) {
		if(board->is_human_player()) {
			board->send_to_socket("draw");
		}
		else {
			run_message_dialog(gobj(), _("Warning"), GTK_MESSAGE_WARNING,
					GTK_BUTTONS_OK,
					_("You may only take a back move when it's your turn. Please wait a minute!"));
		}
	}
}

void MainWindow::on_rue_game()
{
	if(board->is_fight_to_robot()) {
		if(board->is_human_player()) {
			board->rue_move();
			board->rue_move();
		}
		else {
			run_message_dialog(gobj(), _("Warning"), GTK_MESSAGE_WARNING,
					GTK_BUTTONS_OK,
					_("You can only take a back move when it's your turn. Please wait a minute!"));
		}
	}
	else if(board->is_network_game()) {
		if(board->is_human_player()) {
			board->send_to_socket("rue");
		}
		else {
			run_message_dialog(gobj(), _("Warning"), GTK_MESSAGE_WARNING,
					GTK_BUTTONS_OK,
					_("You can only take a back move when it's your turn. Please wait a minute!"));
		}
	}
}

bool MainWindow::on_end_game(OVERSTATUS over)
{
	std::string msg;
	switch(over) {
		case ROBOT_WIN:
			msg = _("You Lose!\nDo you want to start a new game?");
			CSound::play(SND_LOSS);
			break;
		case ROBOT_LOSE:
			msg = _("Congratuations, YOU WIN!\nTo start a new game, click OK");
			CSound::play(SND_WIN);
			break;
		case ROBOT_DRAW:
			msg = _("Draw Game!");
			break;
		case ROBOT_OVER_TIME:
			msg = _("The opponent used up their time. You Win!");
			CSound::play(SND_WIN);
			break;
		case HUMAN_OVER_TIME:
			msg = _("You used up your time. You Lose!");
			CSound::play(SND_LOSS);
			break;
		default:
			break;
	}

	run_message_dialog(gobj(), _("Game Over"), GTK_MESSAGE_INFO, GTK_BUTTONS_OK, msg);

	if(board->is_network_game())
		auto_save_chess_file();
	board->free_game(false);
	set_status();
	return true;
}

void MainWindow::set_red_war_time(const std::string& f_time, const std::string& c_time)
{
	gtk_label_set_text(p2_war_time, f_time.c_str());
	gtk_label_set_text(p2_step_time, c_time.c_str());
}

void MainWindow::set_black_war_time(const std::string& f_time, const std::string& c_time)
{
	gtk_label_set_text(p1_war_time, f_time.c_str());
	gtk_label_set_text(p1_step_time, c_time.c_str());
}

void MainWindow::watch_socket(int fd)
{
	board->watch_socket(fd);
}

void MainWindow::start_with(const std::string& param)
{
	if((param.find("network-game-red,")) != std::string::npos) {
		std::string enemy_name;
		std::string my_name;
		size_t pos_s = param.find("enemy_name:");
		size_t pos_e = param.find(",my_name:");
		size_t pos_m = param.find_first_of("@");
		enemy_name = param.substr(pos_s + 11, pos_m - pos_s - 11);
		pos_m = param.find_last_of("@");
		my_name = param.substr(pos_e + 9, pos_m - pos_e - 9);
		on_network_game(enemy_name, my_name, true);
	}
	else if((param.find("network-game-black,")) != std::string::npos) {
		std::string enemy_name;
		std::string my_name;
		size_t pos_s = param.find("enemy_name:");
		size_t pos_e = param.find(",my_name:");
		size_t pos_m = param.find_first_of("@");
		enemy_name = param.substr(pos_s + 11, pos_m - pos_s - 11);
		pos_m = param.find_last_of("@");
		my_name = param.substr(pos_e + 9, pos_m - pos_e - 9);
		on_network_game(my_name, enemy_name, false);
	}
	else {
		if(!param.empty())
			open_file(param);
	}
}
