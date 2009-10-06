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
	vbox_xml = Gtk::Builder::create_from_file(conf_ui,"conf_vbox");
	if(!vbox_xml)
		exit(271);

	Gtk::VBox * vBox = 0;
	vbox_xml->get_widget("conf_vbox", vBox);

#if 0
	Glib::RefPtr<Glib::Object > adjust =
	vbox_xml->get_object("adjustment_depth");
	Gtk::SpinButton* spinbt =0;
	vbox_xml->get_widget("sb_depth",spinbt);
	spinbt->set_adjustment(adjust);
#endif

	this->add(*vBox);
	this->set_transient_for(*parent);
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
	parent->on_conf_window_close();
}

bool ConfWindow::on_delete_event(GdkEventAny*)
{
	on_button_cancel();

}
