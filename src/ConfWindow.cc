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
#include "MainWindow.h"

ConfWindow::ConfWindow(MainWindow * parent_):parent(parent_)
{
	this->set_size_request(521,577);
	show_all();

}

ConfWindow::~ConfWindow()
{
}

void ConfWindow::on_button_save()
{
	on_button_cancel();
}


void ConfWindow::on_button_cancel()
{
	//delete this;
	//parent->on_conf_window_close(this);
}

