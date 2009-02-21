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

void MainWindow::onNextMove(){}
void MainWindow::onBackMove(){}



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

}

void MainWindow:: on_menu_view_preferences()
{

}

void MainWindow:: on_menu_help_about()
{

}
