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
#include <gtkmm/treeselection.h>

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
"		<menu action='GameMenu'>"
"			<menuitem action='WarAI'/>"
"			<menuitem action='FreePlay'/>"
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

	 btn_start = dynamic_cast<Gtk::Button*>(ui_xml->get_widget("button_start"));
	 btn_end = dynamic_cast<Gtk::Button*>(ui_xml->get_widget("button_end"));
	 btn_prev = dynamic_cast<Gtk::Button*>(ui_xml->get_widget("button_prev"));
	 btn_next = dynamic_cast<Gtk::Button*>(ui_xml->get_widget("button_next"));


	 btn_begin = dynamic_cast<Gtk::Button*>(ui_xml->get_widget("button_begin"));
	 btn_lose  = dynamic_cast<Gtk::Button*>(ui_xml->get_widget("button_lose"));
	 btn_huo   = dynamic_cast<Gtk::Button*>(ui_xml->get_widget("button_huo"));
	 btn_undo  = dynamic_cast<Gtk::Button*>(ui_xml->get_widget("button_undo"));

	btn_start->signal_clicked().connect(
			sigc::mem_fun(*this,&MainWindow::on_start_move));
	btn_end->signal_clicked().connect(
			sigc::mem_fun(*this,&MainWindow::on_end_move));
	btn_prev->signal_clicked().connect(
			sigc::mem_fun(*this,&MainWindow::on_back_move));
	btn_next->signal_clicked().connect(
			sigc::mem_fun(*this,&MainWindow::on_next_move));

	btn_begin->signal_clicked().connect(
			sigc::mem_fun(*this, &MainWindow::on_begin_game));
	btn_lose->signal_clicked().connect(
			sigc::mem_fun(*this, &MainWindow::on_lose_game));
	btn_huo->signal_clicked().connect(
			sigc::mem_fun(*this, &MainWindow::on_huo_game));
	btn_undo->signal_clicked().connect(
			sigc::mem_fun(*this, &MainWindow::on_undo_game));

	board= Gtk::manage(new Board(*this));
	box_board->pack_start(*board);

	this->add(*main_window);
	this->set_title("GMChess");

	/** 设置菜单*/
	init_ui_manager();
	 menubar = ui_manager->get_widget("/MenuBar");
	Gtk::VBox* menu_tool_box = dynamic_cast<Gtk::VBox*>
		(ui_xml->get_widget("box_menu_toolbar"));
	menu_tool_box->pack_start(*menubar,true,true);

	/** 设置Treeview区*/
	Gtk::ScrolledWindow* scrolwin=dynamic_cast<Gtk::ScrolledWindow*>
		(ui_xml->get_widget("scrolledwindow"));
	m_refTreeModel = Gtk::ListStore::create(m_columns);
	m_treeview.set_model( m_refTreeModel);
	scrolwin->add(m_treeview);
	m_treeview.append_column(_("number"),m_columns.step_num);;
	m_treeview.append_column(_("step"),m_columns.step_line);
	m_treeview.set_events(Gdk::BUTTON_PRESS_MASK);
	m_treeview.signal_button_press_event().connect(sigc::mem_fun(*this,
				&MainWindow::on_treeview_click),false);

	set_move_button_property();
	show_all();

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

	//Game menu:
	action_group->add(Gtk::Action::create("GameMenu",_("_Game")));
	action_group->add(Gtk::Action::create("WarAI",Gtk::Stock::MEDIA_PLAY,_("_Fight to AI")),
			sigc::mem_fun(*this, &MainWindow::on_menu_war_to_ai));
	action_group->add(Gtk::Action::create("FreePlay",Gtk::Stock::MEDIA_PLAY,_("Free Play")),
			sigc::mem_fun(*this, &MainWindow::on_menu_free_play));
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
	Gtk::FileChooserDialog dlg(*this,
			_("Choose File"), 
			Gtk::FILE_CHOOSER_ACTION_SAVE);
	dlg.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	dlg.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

	//Gtk::FileFilter filter_media;
	//filter_media.set_name();
	if (Gtk::RESPONSE_OK == dlg.run()) {
		Glib::ustring filename = dlg.get_filename();
		if (filename.empty())
			return;
		Glib::ustring filtername = Glib::ustring("\"")+filename+"\"";
		DLOG("播放 %s\n",filtername.c_str());
		int out = board->open_file( filtername);
		if(out){
			DLOG("open file :%s error\n",filtername.c_str());
		}
		else
			init_move_treeview();
		set_information();

	}
}

void MainWindow::on_menu_war_to_ai()
{

}

void MainWindow::on_menu_free_play()
{

}

void MainWindow::on_menu_file_quit()
{
	Gtk::Main::quit();

}

void MainWindow::on_menu_view_preferences()
{

}

void MainWindow::on_menu_help_about()
{

}

void MainWindow::add_step_line(int num,const Glib::ustring& f_line)
{
	Gtk::TreeModel::iterator iter = m_refTreeModel->append();
	(*iter)[m_columns.step_line] = f_line;
	(*iter)[m_columns.step_num] = num;

}

void MainWindow::init_move_treeview()
{
	m_refTreeModel->clear();

	const std::vector<Glib::ustring>&  move_chinese = board->get_move_chinese_snapshot();
	std::vector<Glib::ustring>::const_iterator iter;
	iter = move_chinese.begin();
	for(int i=1;iter != move_chinese.end(); iter++,i++)
		add_step_line(i,*iter);

}

bool MainWindow::on_treeview_click(GdkEventButton* ev)
{
	Glib::RefPtr<Gtk::TreeSelection> selection = m_treeview.get_selection();
	Gtk::TreeModel::iterator iter  = selection->get_selected();
	if(!selection->count_selected_rows())
		return false;
	Gtk::TreeModel::Path path(iter);
	Gtk::TreeViewColumn* tvc;
	int cx, cy;

	if(!m_treeview.get_path_at_pos((int) ev->x, (int) ev->y, path, tvc, cx, cy))
		return false;;

	if(ev->type == GDK_2BUTTON_PRESS){
		int num = (*iter)[m_columns.step_num];
		board->get_board_by_move(num);

		return true;
	}

	return false;
}



void MainWindow::set_information()
{
	Gtk::Label* p1_name= dynamic_cast<Gtk::Label*>(ui_xml->get_widget("P1_name"));
	Gtk::Label* p2_name= dynamic_cast<Gtk::Label*>(ui_xml->get_widget("P2_name"));
	Gtk::Label* info   = dynamic_cast<Gtk::Label*>(ui_xml->get_widget("info_label"));
	
	const Board_info& board_info = board->get_board_info();
	p1_name->set_label(board_info.black);
	p2_name->set_label(board_info.red);
	Glib::ustring text = Glib::ustring("赛事: ") +board_info.event+"\n";
	text = text + "时间: "+board_info.date+"\n";
	text = text + "地点: "+board_info.site+"\n";
	text = text + "结果: "+board_info.result+"\n";
	text = text + "开局信息: "+board_info.opening+"\n";
	text = text + "变着:  "+board_info.variation+"\n";
	info->set_label(text);
	info->set_ellipsize(Pango::ELLIPSIZE_END);
	set_move_button_property();

}


void MainWindow::set_move_button_property()
{
	int f_status = board->get_status();
	bool f_use=1;
	if( READ_STATUS != f_status){
		f_use = 0;
	}
	btn_next->set_sensitive(f_use);
	btn_prev->set_sensitive(f_use);
	btn_start->set_sensitive(f_use);
	btn_end->set_sensitive(f_use);

}

void MainWindow::on_begin_game()
{

}

void MainWindow::on_lose_game()
{

}

void MainWindow::on_huo_game()
{

}

void MainWindow::on_undo_game()
{

}
