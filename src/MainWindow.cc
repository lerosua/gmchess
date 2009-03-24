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
#include <gtkmm/aboutdialog.h>

#define version "0.10"

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
"			<menuitem action='Help'/>"
"			<menuitem action='About'/>"
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
	buttonbox_war = dynamic_cast<Gtk::ButtonBox*>(ui_xml->get_widget("hbuttonbox_war"));
	text_comment = dynamic_cast<Gtk::TextView*>(ui_xml->get_widget("textview_comment"));

	 btn_start = dynamic_cast<Gtk::Button*>(ui_xml->get_widget("button_start"));
	 btn_end = dynamic_cast<Gtk::Button*>(ui_xml->get_widget("button_end"));
	 btn_prev = dynamic_cast<Gtk::Button*>(ui_xml->get_widget("button_prev"));
	 btn_next = dynamic_cast<Gtk::Button*>(ui_xml->get_widget("button_next"));


	 btn_begin = dynamic_cast<Gtk::Button*>(ui_xml->get_widget("button_begin"));
	 btn_lose  = dynamic_cast<Gtk::Button*>(ui_xml->get_widget("button_lose"));
	 btn_draw   = dynamic_cast<Gtk::Button*>(ui_xml->get_widget("button_draw"));
	 btn_rue  = dynamic_cast<Gtk::Button*>(ui_xml->get_widget("button_rue"));

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
	btn_draw->signal_clicked().connect(
			sigc::mem_fun(*this, &MainWindow::on_draw_game));
	btn_rue->signal_clicked().connect(
			sigc::mem_fun(*this, &MainWindow::on_rue_game));

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

void MainWindow::set_comment(const std::string& f_comment)
{
	text_comment->set_wrap_mode(Gtk::WRAP_WORD);
	Glib::RefPtr<Gtk::TextBuffer> buffer = text_comment->get_buffer();
	buffer->set_text(f_comment);

}
void MainWindow::show_treeview_step()
{
	Glib::RefPtr<Gtk::TreeModel> model= m_treeview.get_model();
	model->foreach_iter(sigc::mem_fun(*this, &MainWindow::on_foreach_iter));

}

bool MainWindow::on_foreach_iter(const Gtk::TreeModel::iterator iter)
{
	int n_step = (*iter)[m_columns.step_num];
	int m_step = board->get_step();
	if(n_step == m_step){
		Glib::RefPtr<Gtk::TreeSelection> sel = m_treeview.get_selection();
		Gtk::TreeModel::Path path(iter);
		m_treeview.scroll_to_row(path);
		sel->select(iter);

		return true;
	}
	return false;


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
void MainWindow::on_start_move()
{
	board->start_move();
	m_treeview.scroll_to_point(1,1);
}
void MainWindow::on_end_move()
{
	board->end_move();
	show_treeview_step();
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
	action_group->add(Gtk::Action::create("Help", Gtk::Stock::HELP),
			sigc::mem_fun(*this, &MainWindow::on_menu_help));
	action_group->add(Gtk::Action::create("About", Gtk::Stock::ABOUT),
			sigc::mem_fun(*this, &MainWindow::on_menu_about));

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

	Gtk::FileFilter filter_pgn;
	filter_pgn.set_name("PGN");
	filter_pgn.add_pattern("*.pgn");
	dlg.add_filter(filter_pgn);
	/** 中游象棋*/
	Gtk::FileFilter filter_ccm;
	filter_ccm.set_name("CCM");
	filter_ccm.add_pattern("*.ccm");
	dlg.add_filter(filter_ccm);
	/** QQ象棋*/
	Gtk::FileFilter filter_che;
	filter_che.set_name("CHE");
	filter_che.add_pattern("*.che");
	dlg.add_filter(filter_che);
	/** 联众象棋*/
	Gtk::FileFilter filter_chn;
	filter_chn.set_name("CHN");
	filter_chn.add_pattern("*.chn");
	dlg.add_filter(filter_chn);

	/** 弈天象棋*/
	Gtk::FileFilter filter_mxq;
	filter_mxq.set_name("MXQ");
	filter_mxq.add_pattern("*.mxq");
	dlg.add_filter(filter_mxq);
	/** 象棋演播室*/
	Gtk::FileFilter filter_xqf;
	filter_xqf.set_name("MXQ");
	filter_xqf.add_pattern("*.mxq");
	dlg.add_filter(filter_xqf);
	/** 所有文件*/
	Gtk::FileFilter filter_any;
	filter_any.set_name("All Files");
	filter_any.add_pattern("*");
	dlg.add_filter(filter_any);
	
	std::string filename ;
	if (Gtk::RESPONSE_OK == dlg.run()) {
		filename = dlg.get_filename();
	}
	if (filename.empty())
		return;
	/** 获取文件后先它将它转换成pgn文件才能打开*/
		int out;

		size_t pos = filename.find(".pgn");
		if(pos == std::string::npos){
			char cmd[1024];
			sprintf(cmd,"convert_pgn \"%s\" /tmp/gmchess.pgn",filename.c_str());
			if(system(cmd)<0){
				DLOG("convert pgn file error\n");
				return;
			}

			out = board->open_file("/tmp/gmchess.pgn");
		}
		else
			out = board->open_file( filename);
		if(out<0){
			DLOG("open file :%s error\n",filename.c_str());
			Gtk::MessageDialog dialog("Error",false,
					Gtk::MESSAGE_INFO);
			dialog.set_secondary_text(_("the file maybe not right format for chess"));
			dialog.run();
		}
		else{
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

void MainWindow::on_menu_help()
{

}

void MainWindow::on_menu_about()
{
	static Gtk::AboutDialog*  about(0);
	if(about == 0){
		std::vector<Glib::ustring> authors;
		authors.push_back("lerosua ");
		authors.push_back("wind");
		about = new Gtk::AboutDialog ;
		Glib::RefPtr<Gdk::Pixbuf> logo  = Gdk::Pixbuf::create_from_file(DATA_DIR"/gmchess.png");
		about->set_logo(logo);
		about->set_program_name("GMChess");
		about->set_version(version);
		about->set_website("http://lerosua.org");
		//about->set_website_label("lerosua");
		about->set_copyright("Copyright (c) 2009 lerosua");
		about->set_comments(_("GMChess is chinese chess game write by gtkmm"));
		about->set_authors(authors);
		about->set_license (_("This program is licenced under GNU General Public Licence (GPL) version 2."));
		about->set_translator_credits("lerosua");


	}
	about->show();
	about->raise();

}

void MainWindow::add_step_line(int num,const Glib::ustring& f_line)
{
	Gtk::TreeModel::iterator iter = m_refTreeModel->append();
	(*iter)[m_columns.step_line] = f_line;
	(*iter)[m_columns.step_num] = num;

}

void MainWindow::del_step_last_line()
{
	Gtk::TreeModel::iterator iter = m_refTreeModel->children().end();
	iter--;
	m_refTreeModel->erase(*iter);

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
	Glib::ustring text = Glib::ustring(_("Game: ")) +board_info.event+"\n";
	text = text + _("Time: ")+board_info.date+"\n";
	text = text + _("Site: ")+board_info.site+"\n";
	text = text + _("Result: ")+board_info.result+"\n";
	text = text + _("Opening: ")+board_info.opening+"\n";
	text = text + _("Variation:  ")+board_info.variation+"\n";
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

/** draw 是打平局面的意思*/
void MainWindow::on_draw_game()
{

}

void MainWindow::on_rue_game()
{
	/** 如果是网络对战，则需要确认信息*/
	board->rue_move();

}
