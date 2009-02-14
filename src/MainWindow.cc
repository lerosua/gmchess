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

MainWindow::MainWindow()
{
	board= Gtk::manage(new Board());
	this->add(*board);

	this->set_title("GMChess");



}
MainWindow::~MainWindow()
{
}

