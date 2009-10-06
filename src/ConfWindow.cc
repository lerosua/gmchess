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

	Gtk::SpinButton* spinbt =0;
	vbox_xml->get_widget("sb_depth",spinbt);
	Gtk::Adjustment* adjust = spinbt->get_adjustment();
	adjust->set_lower(1.0);
	adjust->set_upper(15.0);
	adjust->set_step_increment(1.0);




	Gtk::Button* bt = 0;
	vbox_xml->get_widget("button_ok", bt);
	bt->signal_clicked().connect(sigc::mem_fun(*this,&ConfWindow::on_button_save));
	bt = 0;
	vbox_xml->get_widget("button_cancel", bt);
	bt->signal_clicked().connect(sigc::mem_fun(*this,&ConfWindow::on_button_cancel));


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
	signal_quit_.emit();
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
