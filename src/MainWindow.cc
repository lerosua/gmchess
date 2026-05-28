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

static GtkWidget* create_message_dialog(GtkWindow* parent, const char* title,
		GtkMessageType type, GtkButtonsType buttons, const std::string& secondary)
{
	GtkWidget* dialog = gtk_message_dialog_new(parent, GTK_DIALOG_MODAL,
			type, buttons, "%s", title);
	if(!secondary.empty())
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
				"%s", secondary.c_str());
	return dialog;
}

static gint run_message_dialog(GtkWindow* parent, const char* title,
		GtkMessageType type, GtkButtonsType buttons, const std::string& secondary)
{
	GtkWidget* dialog = create_message_dialog(parent, title, type, buttons, secondary);
	gint result = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	return result;
}

static GtkFileFilter* add_file_filter(GtkFileChooser* chooser, const char* name,
		const char* lower, const char* upper)
{
	GtkFileFilter* filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, name);
	gtk_file_filter_add_pattern(filter, lower);
	gtk_file_filter_add_pattern(filter, upper);
	gtk_file_chooser_add_filter(chooser, filter);
	return filter;
}

MainWindow::MainWindow()
	: board(NULL)
	, ui_xml(NULL)
	, window(NULL)
	, menubar(NULL)
	, m_treeview(NULL)
	, m_refTreeModel(NULL)
	, m_bookview(NULL)
	, buttonbox_war(NULL)
	, text_comment(NULL)
	, text_engine_log(NULL)
	, m_notebook(NULL)
	, confwindow(NULL)
	, ui_logo(NULL)
{
	ui_xml = gtk_builder_new_from_file(main_ui);
	if(!ui_xml)
		exit(271);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(window, "delete-event", G_CALLBACK(delete_event_cb), this);
	g_signal_connect(window, "destroy", G_CALLBACK(window_destroy_cb), this);

	GtkWidget* main_box = builder_widget("main_window");
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
	gtk_box_pack_start(GTK_BOX(box_board), board->widget(), TRUE, TRUE, 0);

	gtk_container_add(GTK_CONTAINER(window), main_box);
	gtk_window_set_title(GTK_WINDOW(window), _("GMChess"));

	GError* error = NULL;
	ui_logo = gdk_pixbuf_new_from_file(DATA_DIR"/gmchess.png", &error);
	if(ui_logo)
		gtk_window_set_icon(GTK_WINDOW(window), ui_logo);
	if(error)
		g_error_free(error);

	init_ui_manager();
	GtkWidget* menu_tool_box = builder_widget("box_menu_toolbar");
	gtk_box_pack_start(GTK_BOX(menu_tool_box), menubar, FALSE, FALSE, 0);

	init_move_treeview();

	GtkWidget* scroll_book = builder_widget("scrolledwin_book");
	m_bookview = new BookView(this);
	gtk_container_add(GTK_CONTAINER(scroll_book), m_bookview->widget());

	p1_image = GTK_IMAGE(builder_widget("image_p1"));
	p2_image = GTK_IMAGE(builder_widget("image_p2"));
	p1_war_time = GTK_LABEL(builder_widget("P1_war_time"));
	p2_war_time = GTK_LABEL(builder_widget("P2_war_time"));
	p1_step_time = GTK_LABEL(builder_widget("P1_step_time"));
	p2_step_time = GTK_LABEL(builder_widget("P2_step_time"));
	p1_name = GTK_LABEL(builder_widget("P1_name"));
	p2_name = GTK_LABEL(builder_widget("P2_name"));

	init_conf();
	g_signal_connect(board->widget(), "size-allocate", G_CALLBACK(size_allocate_cb), this);

	gtk_widget_show_all(window);
	gtk_widget_hide(GTK_WIDGET(p1_image));
	gtk_widget_hide(GTK_WIDGET(p2_image));

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
		gtk_widget_destroy(window);
	delete confwindow;
	delete m_bookview;
	delete board;
	if(m_refTreeModel)
		g_object_unref(m_refTreeModel);
	if(ui_logo)
		g_object_unref(ui_logo);
	if(ui_xml)
		g_object_unref(ui_xml);
}

GtkWidget* MainWindow::builder_widget(const char* name)
{
	GObject* object = gtk_builder_get_object(ui_xml, name);
	if(!object)
		g_error("Unable to find widget '%s' in %s", name, main_ui);
	return GTK_WIDGET(object);
}

GtkWidget* MainWindow::create_menu_item(const char* label, GCallback callback)
{
	GtkWidget* item = gtk_menu_item_new_with_mnemonic(label);
	g_signal_connect(item, "activate", callback, this);
	return item;
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

gboolean MainWindow::tree_button_cb(GtkWidget*, GdkEventButton* event, gpointer data)
{
	return static_cast<MainWindow*>(data)->on_treeview_click(event);
}

gboolean MainWindow::delete_event_cb(GtkWidget*, GdkEvent* event, gpointer data)
{
	return static_cast<MainWindow*>(data)->on_delete_event((GdkEventAny*)event);
}

void MainWindow::window_destroy_cb(GtkWidget*, gpointer data)
{
	static_cast<MainWindow*>(data)->window = NULL;
	gtk_main_quit();
}

void MainWindow::size_allocate_cb(GtkWidget*, GtkAllocation*, gpointer data)
{
	static_cast<MainWindow*>(data)->on_size_change();
}

void MainWindow::menu_open_cb(GtkMenuItem*, gpointer data)
{
	static_cast<MainWindow*>(data)->on_menu_open_file();
}

void MainWindow::menu_save_cb(GtkMenuItem*, gpointer data)
{
	static_cast<MainWindow*>(data)->on_menu_save_file();
}

void MainWindow::menu_save_board_cb(GtkMenuItem*, gpointer data)
{
	static_cast<MainWindow*>(data)->on_menu_save_board_to_png();
}

void MainWindow::menu_quit_cb(GtkMenuItem*, gpointer data)
{
	static_cast<MainWindow*>(data)->on_menu_file_quit();
}

void MainWindow::menu_preferences_cb(GtkMenuItem*, gpointer data)
{
	static_cast<MainWindow*>(data)->on_menu_view_preferences();
}

void MainWindow::menu_war_ai_cb(GtkMenuItem*, gpointer data)
{
	static_cast<MainWindow*>(data)->on_menu_war_to_ai();
}

void MainWindow::menu_free_play_cb(GtkMenuItem*, gpointer data)
{
	static_cast<MainWindow*>(data)->on_menu_free_play();
}

void MainWindow::menu_rev_play_cb(GtkMenuItem*, gpointer data)
{
	static_cast<MainWindow*>(data)->on_menu_rev_play();
}

void MainWindow::menu_help_cb(GtkMenuItem*, gpointer data)
{
	static_cast<MainWindow*>(data)->on_menu_help();
}

void MainWindow::menu_about_cb(GtkMenuItem*, gpointer data)
{
	static_cast<MainWindow*>(data)->on_menu_about();
}

void MainWindow::on_size_change()
{
	board->configure_board(gtk_widget_get_allocated_width(board->widget()));
}

void MainWindow::change_play(bool player)
{
	if(player) {
		gtk_widget_hide(GTK_WIDGET(p1_image));
		gtk_widget_show(GTK_WIDGET(p2_image));
	}
	else {
		gtk_widget_hide(GTK_WIDGET(p2_image));
		gtk_widget_show(GTK_WIDGET(p1_image));
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
	GtkTreeModel* model = GTK_TREE_MODEL(m_refTreeModel);
	GtkTreeIter iter;
	if(!gtk_tree_model_get_iter_first(model, &iter))
		return;

	int current_step = board->get_step();
	do {
		gint row_step = 0;
		gtk_tree_model_get(model, &iter, COL_STEP_NUM, &row_step, -1);
		if(row_step == current_step) {
			GtkTreePath* path = gtk_tree_model_get_path(model, &iter);
			gtk_tree_view_scroll_to_cell(m_treeview, path, NULL, FALSE, 0, 0);
			GtkTreeSelection* sel = gtk_tree_view_get_selection(m_treeview);
			gtk_tree_selection_select_iter(sel, &iter);
			gtk_tree_path_free(path);
			return;
		}
	} while(gtk_tree_model_iter_next(model, &iter));
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
	gtk_tree_view_scroll_to_point(m_treeview, 1, 1);
}

void MainWindow::on_last_move()
{
	board->last_move();
	show_treeview_step();
}

void MainWindow::init_ui_manager()
{
	menubar = gtk_menu_bar_new();

	GtkWidget* file_menu = gtk_menu_new();
	GtkWidget* file_root = gtk_menu_item_new_with_mnemonic(_("_File"));
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_root), file_menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), create_menu_item(_("Open file"), G_CALLBACK(menu_open_cb)));
	gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), create_menu_item(_("Save as"), G_CALLBACK(menu_save_cb)));
	gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), create_menu_item(_("Save Board"), G_CALLBACK(menu_save_board_cb)));
	gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), gtk_separator_menu_item_new());
	gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), create_menu_item(_("_Quit"), G_CALLBACK(menu_quit_cb)));
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), file_root);

	GtkWidget* view_menu = gtk_menu_new();
	GtkWidget* view_root = gtk_menu_item_new_with_mnemonic(_("_View"));
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(view_root), view_menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), create_menu_item(_("_Switch colour"), G_CALLBACK(menu_rev_play_cb)));
	gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), create_menu_item(_("_Preferences"), G_CALLBACK(menu_preferences_cb)));
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), view_root);

	GtkWidget* game_menu = gtk_menu_new();
	GtkWidget* game_root = gtk_menu_item_new_with_mnemonic(_("_Game"));
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(game_root), game_menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(game_menu), create_menu_item(_("_Fight to AI"), G_CALLBACK(menu_war_ai_cb)));
	gtk_menu_shell_append(GTK_MENU_SHELL(game_menu), create_menu_item(_("Free Play"), G_CALLBACK(menu_free_play_cb)));
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), game_root);

	GtkWidget* help_menu = gtk_menu_new();
	GtkWidget* help_root = gtk_menu_item_new_with_mnemonic(_("_Help"));
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(help_root), help_menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), create_menu_item(_("_Help"), G_CALLBACK(menu_help_cb)));
	gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), create_menu_item(_("_About"), G_CALLBACK(menu_about_cb)));
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), help_root);
}

void MainWindow::on_menu_save_board_to_png()
{
	board->save_board_to_file("/tmp/1.png");
}

void MainWindow::on_menu_save_file()
{
	GtkWidget* dlg = gtk_file_chooser_dialog_new(_("Save File"), gobj(),
			GTK_FILE_CHOOSER_ACTION_SAVE,
			_("_Cancel"), GTK_RESPONSE_CANCEL,
			_("_Save"), GTK_RESPONSE_OK,
			NULL);

	std::string filename;
	if(gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_OK) {
		char* chosen = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg));
		if(chosen) {
			filename = chosen;
			g_free(chosen);
		}
	}
	gtk_widget_destroy(dlg);
	if(filename.empty())
		return;

	std::ofstream file(filename.c_str());
	if(!file) {
		DLOG("open %s file error\n", filename.c_str());
		return;
	}

	GtkTreeModel* model = GTK_TREE_MODEL(m_refTreeModel);
	GtkTreeIter iter;
	gboolean valid = gtk_tree_model_get_iter_first(model, &iter);
	while(valid) {
		gint bout = 0;
		gchar* line = NULL;
		gtk_tree_model_get(model, &iter, COL_STEP_BOUT, &bout, COL_STEP_LINE, &line, -1);
		file << bout << ". " << (line ? line : "");
		g_free(line);

		valid = gtk_tree_model_iter_next(model, &iter);
		if(valid) {
			gtk_tree_model_get(model, &iter, COL_STEP_LINE, &line, -1);
			file << "  " << (line ? line : "") << std::endl;
			g_free(line);
			valid = gtk_tree_model_iter_next(model, &iter);
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

	GtkTreeModel* model = GTK_TREE_MODEL(m_refTreeModel);
	GtkTreeIter iter;
	gboolean valid = gtk_tree_model_get_iter_first(model, &iter);
	while(valid) {
		gint bout = 0;
		gchar* line = NULL;
		gtk_tree_model_get(model, &iter, COL_STEP_BOUT, &bout, COL_STEP_LINE, &line, -1);
		file << bout << ". " << (line ? line : "");
		g_free(line);

		valid = gtk_tree_model_iter_next(model, &iter);
		if(valid) {
			gtk_tree_model_get(model, &iter, COL_STEP_LINE, &line, -1);
			file << "  " << (line ? line : "") << std::endl;
			g_free(line);
			valid = gtk_tree_model_iter_next(model, &iter);
		}
		else {
			file << std::endl;
		}
	}
	file.close();
}

void MainWindow::on_menu_open_file()
{
	GtkWidget* dlg = gtk_file_chooser_dialog_new(_("Choose File"), gobj(),
			GTK_FILE_CHOOSER_ACTION_OPEN,
			_("_Cancel"), GTK_RESPONSE_CANCEL,
			_("_Open"), GTK_RESPONSE_OK,
			NULL);
	GtkFileChooser* chooser = GTK_FILE_CHOOSER(dlg);
	add_file_filter(chooser, "PGN", "*.pgn", "*.PGN");
	add_file_filter(chooser, "CCM", "*.ccm", "*.CCM");
	add_file_filter(chooser, "CHE", "*.che", "*.CHE");
	add_file_filter(chooser, "CHN", "*.chn", "*.CHN");
	add_file_filter(chooser, "MXQ", "*.mxq", "*.MXQ");
	add_file_filter(chooser, "XQF", "*.xqf", "*.XQF");
	GtkFileFilter* filter_any = gtk_file_filter_new();
	gtk_file_filter_set_name(filter_any, _("All Files"));
	gtk_file_filter_add_pattern(filter_any, "*");
	gtk_file_chooser_add_filter(chooser, filter_any);

	std::string filename;
	if(gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_OK) {
		char* chosen = gtk_file_chooser_get_filename(chooser);
		if(chosen) {
			filename = chosen;
			g_free(chosen);
		}
	}
	gtk_widget_destroy(dlg);
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
	gtk_list_store_clear(m_refTreeModel);
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
		gtk_widget_destroy(window);
}

gboolean MainWindow::on_delete_event(GdkEventAny*)
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
	gtk_dialog_run(GTK_DIALOG(about));
	gtk_widget_destroy(about);
}

void MainWindow::add_step_line(int num, const std::string& f_line)
{
	GtkTreeIter iter;
	gtk_list_store_append(m_refTreeModel, &iter);
	gtk_list_store_set(m_refTreeModel, &iter,
			COL_STEP_LINE, f_line.c_str(),
			COL_STEP_NUM, num,
			COL_STEP_BOUT, (int)((num + 1) / 2),
			COL_PLAYER, (num % 2) == 0 ? _("Black") : _("Red"),
			-1);

	GtkTreePath* path = gtk_tree_model_get_path(GTK_TREE_MODEL(m_refTreeModel), &iter);
	gtk_tree_view_scroll_to_cell(m_treeview, path, NULL, FALSE, 0, 0);
	gtk_tree_path_free(path);
}

void MainWindow::del_step_last_line()
{
	GtkTreeModel* model = GTK_TREE_MODEL(m_refTreeModel);
	GtkTreeIter iter;
	if(!gtk_tree_model_get_iter_first(model, &iter))
		return;

	GtkTreeIter last = iter;
	while(gtk_tree_model_iter_next(model, &iter))
		last = iter;
	gtk_list_store_remove(m_refTreeModel, &last);
}

void MainWindow::init_move_treeview()
{
	if(m_treeview == NULL) {
		GtkWidget* scrolwin = builder_widget("scrolledwindow");
		m_refTreeModel = gtk_list_store_new(N_STEP_COLUMNS,
				G_TYPE_INT, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING);
		m_treeview = GTK_TREE_VIEW(gtk_tree_view_new());
		gtk_tree_view_set_model(m_treeview, GTK_TREE_MODEL(m_refTreeModel));
		gtk_container_add(GTK_CONTAINER(scrolwin), GTK_WIDGET(m_treeview));
		gtk_tree_view_insert_column_with_attributes(m_treeview,
				-1, _("Turn"), gtk_cell_renderer_text_new(), "text", COL_STEP_BOUT, NULL);
		gtk_tree_view_insert_column_with_attributes(m_treeview,
				-1, "  ", gtk_cell_renderer_text_new(), "text", COL_PLAYER, NULL);
		gtk_tree_view_insert_column_with_attributes(m_treeview,
				-1, _("Move"), gtk_cell_renderer_text_new(), "text", COL_STEP_LINE, NULL);
		gtk_widget_add_events(GTK_WIDGET(m_treeview), GDK_BUTTON_PRESS_MASK);
		g_signal_connect(m_treeview, "button-press-event", G_CALLBACK(tree_button_cb), this);
		gtk_widget_show(GTK_WIDGET(m_treeview));
		set_status();
		return;
	}

	gtk_list_store_clear(m_refTreeModel);
	const std::vector<std::string>& move_chinese = board->get_move_chinese_snapshot();
	for(std::vector<std::string>::const_iterator iter = move_chinese.begin();
			iter != move_chinese.end(); ++iter) {
		add_step_line((int)(iter - move_chinese.begin()) + 1, *iter);
	}
}

gboolean MainWindow::on_treeview_click(GdkEventButton* ev)
{
	if(board->is_fight_to_robot())
		return TRUE;

	GtkTreePath* path = NULL;
	GtkTreeViewColumn* tvc = NULL;
	int cx, cy;
	if(!gtk_tree_view_get_path_at_pos(m_treeview, (int)ev->x, (int)ev->y,
				&path, &tvc, &cx, &cy))
		return FALSE;

	GtkTreeModel* model = GTK_TREE_MODEL(m_refTreeModel);
	GtkTreeIter iter;
	if(!gtk_tree_model_get_iter(model, &iter, path)) {
		gtk_tree_path_free(path);
		return FALSE;
	}

	GtkTreeSelection* selection = gtk_tree_view_get_selection(m_treeview);
	gtk_tree_selection_select_iter(selection, &iter);

	if(ev->type == GDK_2BUTTON_PRESS) {
		gint num = 0;
		gtk_tree_model_get(model, &iter, COL_STEP_NUM, &num, -1);
		board->get_board_by_move(num);
		gtk_tree_path_free(path);
		return TRUE;
	}

	gtk_tree_path_free(path);
	return FALSE;
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

	gtk_list_store_clear(m_refTreeModel);
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
			gtk_list_store_clear(m_refTreeModel);
			board->chanju_game();
		}
		return;
	}
	else if(board->is_network_game()) {
		run_message_dialog(gobj(), _("Information"), GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
				_("You are play online, Please end this game first!"));
		return;
	}
	gtk_list_store_clear(m_refTreeModel);
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
			gtk_list_store_clear(m_refTreeModel);
			board->new_game();
		}
		return;
	}
	else if(board->is_network_game()) {
		run_message_dialog(gobj(), _("Information"), GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
				_("You are playing online, Please end this game first!"));
		return;
	}
	gtk_list_store_clear(m_refTreeModel);
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
