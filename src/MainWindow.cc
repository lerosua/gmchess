/*
 * =====================================================================================
 *
 *       Filename:  MainWindow.cc
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2009年02月14日 07时52分14秒 CST
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  lerosua (), lerosua@gmail.com
 *        Company:  Cyclone
 *
 * =====================================================================================
 */
#include "MainWindow.h"
#include <glib/gi18n.h>
#include <gtkmm/button.h>

Glib::ustring ui_info =
"<ui>"
"	<menubar name='MenuBar'>"
"		<menu action='FileMenu'>"
"			<menuitem action='OpenFile'/>"
"			<separator/>"
"			<menuitem action='FileQuit'/>"
"        	</menu>"
"		<menu action='ViewMenu'>"
"			<menuitem action='ViewPreferences'/>"
"		</menu>"
"		<menu action='HelpMenu'>"
"			<menuitem action='HelpAbout'/>"
"		</menu>"
"	</menubar>"
"</ui>";





MainWindow::MainWindow():menubar(NULL)
{
	ui_xml = Gnome::Glade::Xml::create(main_ui,"main_window");
	if(!ui_xml)
		exit(271);
	Gtk::VBox* main_window = dynamic_cast<Gtk::VBox*>(ui_xml->get_widget("main_window"));

	Gtk::VBox* box_board = dynamic_cast<Gtk::VBox*>(ui_xml->get_widget("vbox_board"));

	Gtk::Button* btn_start = dynamic_cast<Gtk::Button*>(ui_xml->get_widget("button_start"));
	Gtk::Button* btn_end = dynamic_cast<Gtk::Button*>(ui_xml->get_widget("button_end"));
	Gtk::Button* btn_prev = dynamic_cast<Gtk::Button*>(ui_xml->get_widget("button_prev"));
	Gtk::Button* btn_next = dynamic_cast<Gtk::Button*>(ui_xml->get_widget("button_next"));

	btn_start->signal_clicked().connect(
			sigc::mem_fun(*this,&MainWindow::on_start_move));
	btn_end->signal_clicked().connect(
			sigc::mem_fun(*this,&MainWindow::on_end_move));
	btn_prev->signal_clicked().connect(
			sigc::mem_fun(*this,&MainWindow::on_back_move));
	btn_next->signal_clicked().connect(
			sigc::mem_fun(*this,&MainWindow::on_next_move));

	board= Gtk::manage(new Board());
	box_board->add(*board);

	this->add(*main_window);

	init_ui_manager();
	 menubar = ui_manager->get_widget("/MenuBar");

	Gtk::VBox* menu_tool_box = dynamic_cast<Gtk::VBox*>
		(ui_xml->get_widget("box_menu_toolbar"));
	menu_tool_box->pack_start(*menubar,true,true);
	this->set_title("GMChess");



}
MainWindow::~MainWindow()
{
}

void MainWindow::on_next_move()
{
	board->next_move();

}
void MainWindow::on_back_move()
{
	board->back_move();
}
void MainWindow::on_start_move()
{
	board->start_move();
}
void MainWindow::on_end_move()
{
	board->end_move();
}



void MainWindow::init_ui_manager()
{
	if (!action_group)
		action_group = Gtk::ActionGroup::create();
	Glib::RefPtr<Gtk::Action> action ;
	//File menu:
	action_group->add(Gtk::Action::create("FileMenu", _("_File")));

	action = Gtk::Action::create("OpenFile", Gtk::Stock::OPEN,_("Open file"));
	action->set_tooltip(_("Open chessman File"));
	action_group->add(action,
			sigc::mem_fun(*this, &MainWindow::on_menu_open_file));

	action_group->add(Gtk::Action::create("FileQuit", Gtk::Stock::QUIT),
			sigc::mem_fun(*this, &MainWindow::on_menu_file_quit));

	//View menu:
	action_group->add(Gtk::Action::create("ViewMenu", _("_View")));


	action_group->add(Gtk::Action::create("ViewPreferences", Gtk::Stock::PREFERENCES),
			sigc::mem_fun(*this, &MainWindow::on_menu_view_preferences));

	//Help menu:
	action_group->add(Gtk::Action::create("HelpMenu", _("_Help")));
	action_group->add(Gtk::Action::create("HelpAbout", Gtk::Stock::HELP),
			sigc::mem_fun(*this, &MainWindow::on_menu_help_about));

	if (!ui_manager)
		ui_manager = Gtk::UIManager::create();

	ui_manager->insert_action_group(action_group);
	add_accel_group(ui_manager->get_accel_group());
	ui_manager->add_ui_from_string(ui_info);

}


void MainWindow:: on_menu_open_file()
{

}
void MainWindow:: on_menu_file_quit()
{
	printf("gmchess quit\n");
	Gtk::Main::quit();

}

void MainWindow:: on_menu_view_preferences()
{

}

void MainWindow:: on_menu_help_about()
{

}
