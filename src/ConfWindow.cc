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
					     ,m_size_big(true)
					     ,m_depth("10")
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


	std::string& size_big = GMConf["desktop_size"];
	m_size_big = (!size_big.empty())&&(size_big[0]=='1');
	std::string& use_book = GMConf["usebook"];
	m_usebook = (!use_book.empty()) && (use_book[0] == '1'); 
	m_depth = GMConf["engine_depth"] ;

	m_pVariablesMap = new VariablesMap(vbox_xml);
	m_pVariablesMap->connect_widget("b_radiobutton",m_size_big);
	m_pVariablesMap->connect_widget("cb_book",m_usebook);
	m_pVariablesMap->connect_widget("sb_depth",m_depth);

	m_pVariablesMap->transfer_variables_to_widgets();

	this->add(*vBox);
	this->set_transient_for(*parent);
	//this->set_size_request(521,577);
	show_all();

}

ConfWindow::~ConfWindow()
{
	delete m_pVariablesMap;
}

void ConfWindow::on_button_save()
{
	write_to_GMConf();
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
void ConfWindow::write_to_GMConf()
{
	m_pVariablesMap->transfer_widgets_to_variables();
	GMConf["usebook"] = m_usebook? "1":"0";
	GMConf["desktop_size"] = m_size_big?"1":"0";
	GMConf["engine_depth"] = m_depth;
}
