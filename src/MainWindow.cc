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

MainWindow::MainWindow()
{
	ui_xml = Gnome::Glade::Xml::create(main_ui,"main_window");
	if(!ui_xml)
		exit(271);
	Gtk::VBox* main_window = dynamic_cast<Gtk::VBox*>(ui_xml->get_widget("main_window"));

	Gtk::VBox* box_board = dynamic_cast<Gtk::VBox*>(ui_xml->get_widget("vbox_board"));

	board= Gtk::manage(new Board());
	//this->add(*board);
	box_board->add(*board);

	this->add(*main_window);
	this->set_title("GMChess");



}
MainWindow::~MainWindow()
{
}

void MainWindow::onNextMove(){}
void MainWindow::onBackMove(){}
