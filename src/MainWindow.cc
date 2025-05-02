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
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include "BookView.h"
#include "ConfWindow.h"
#include "gmchess.h"
#include "Sound.h"

#define version "0.29.6"

Glib::ustring ui_info =
"<ui>"
"	<menubar name='MenuBar'>"
"		<menu action='FileMenu'>"
"			<menuitem action='OpenFile'/>"
"			<menuitem action='SaveAs'/>"
"			<menuitem action='SaveBoard'/>"
"			<separator/>"
"			<menuitem action='FileQuit'/>"
"        	</menu>"
"		<menu action='ViewMenu'>"
"			<menuitem action='RevPlay'/>"
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
			 ,confwindow(NULL)
{
	ui_xml = Gtk::Builder::create_from_file(main_ui,"main_window");
	if(!ui_xml)
		exit(271);
	Gtk::VBox* main_window =0;
	Gtk::VBox* box_board=0;

	ui_xml->get_widget("main_window",main_window);
	ui_xml->get_widget("vbox_board",box_board);
	ui_xml->get_widget("hbuttonbox_war",buttonbox_war);
	ui_xml->get_widget("textview_comment",text_comment);
	ui_xml->get_widget("textview_engine_log",text_engine_log);
	ui_xml->get_widget("notebook",m_notebook);
	ui_xml->get_widget("button_start",btn_start);
	ui_xml->get_widget("button_end",btn_end);
	ui_xml->get_widget("button_prev",btn_prev);
	ui_xml->get_widget("button_next",btn_next);
	ui_xml->get_widget("button_chanju", btn_chanjue);
	ui_xml->get_widget("button_begin",btn_begin);
	ui_xml->get_widget("button_lose",btn_lose);
	ui_xml->get_widget("button_draw",btn_draw);
	ui_xml->get_widget("button_rue",btn_rue);

	btn_start->signal_clicked().connect(
			sigc::mem_fun(*this,&MainWindow::on_first_move));
	btn_end->signal_clicked().connect(
			sigc::mem_fun(*this,&MainWindow::on_last_move));
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
	btn_chanjue->signal_clicked().connect(
			sigc::mem_fun(*this, &MainWindow::on_chanju_game));

	board = Gtk::manage(new Board(*this));
	box_board->pack_start(*board);

	this->add(*main_window);
	this->set_title("GMChess");

	ui_logo  = Gdk::Pixbuf::create_from_file(DATA_DIR"/gmchess.png");
	this->set_icon(ui_logo);
	/** 设置菜单, set the menu*/
	init_ui_manager();
	menubar = ui_manager->get_widget("/MenuBar");
	Gtk::VBox* menu_tool_box=0;
	ui_xml->get_widget("box_menu_toolbar",menu_tool_box);
	menu_tool_box->pack_start(*menubar,true,true);

	/** 设置Treeview区, set the treeview*/
	Gtk::ScrolledWindow* scrolwin= 0;
	ui_xml->get_widget("scrolledwindow",scrolwin);
	m_refTreeModel = Gtk::ListStore::create(m_columns);
	m_treeview.set_model( m_refTreeModel);
	scrolwin->add(m_treeview);
	m_treeview.append_column(_("bout"),m_columns.step_bout);
	m_treeview.append_column("  ",m_columns.player);
	m_treeview.append_column(_("step"),m_columns.step_line);
	m_treeview.set_events(Gdk::BUTTON_PRESS_MASK);
	m_treeview.signal_button_press_event().connect(sigc::mem_fun(*this,
				&MainWindow::on_treeview_click),false);

	set_status();

	Gtk::ScrolledWindow* scroll_book=0;
	ui_xml->get_widget("scrolledwin_book",scroll_book);
	m_bookview= new BookView(this);
	scroll_book->add(*m_bookview);

	ui_xml->get_widget("image_p1",p1_image);
	ui_xml->get_widget("image_p2",p2_image);
	ui_xml->get_widget("P1_war_time",p1_war_time);
	ui_xml->get_widget("P2_war_time",p2_war_time);
	ui_xml->get_widget("P1_step_time",p1_step_time);
	ui_xml->get_widget("P2_step_time",p2_step_time);
	ui_xml->get_widget("P1_name",p1_name);
	ui_xml->get_widget("P2_name",p2_name);

	init_conf();
	this->signal_check_resize().connect(
			sigc::mem_fun(*this,&MainWindow::on_size_change));
	
#ifdef __linux__
	/** test for rgba , it not work in mac osx*/
	Glib::RefPtr<const Gdk::Colormap> colormap_ = this->get_screen()->get_rgba_colormap();
	this->set_default_colormap(colormap_);
#endif

	show_all();
	p1_image->hide();
	p2_image->hide();

	if(atoi(GMConf["desktop_size"].c_str()) == 1)
		board->set_board_size(BIG_BOARD);
	else
		board->set_board_size(SMALL_BOARD);
	int _depth = atoi(GMConf["engine_depth"].c_str());
	g_log("Mainwindow",G_LOG_LEVEL_INFO,"depth %d",_depth);
	board->set_level_config(_depth,0,0,0,0,0,atoi(GMConf["usebook"].c_str()));
	std::string tmp = GMConf["step_time"];
	if(tmp.empty())
		tmp = "240";
	//int _step_time= atoi(GMConf["step_time"].c_str());
	int _step_time= atoi(tmp.c_str());
	tmp = GMConf["play_time"];
	if(tmp.empty())
		tmp="60";
	//int _play_time= atoi(GMConf["play_time"].c_str());
	int _play_time= atoi(tmp.c_str());
	if(_step_time>0&&_step_time<600 && _play_time>0)
		board->set_time(_step_time,_play_time);
	std::string theme_ = GMConf["themes"];
	std::string engine_name = GMConf["engine_name"];
	std::string color_ = GMConf["line_color"];
	if(engine_name.empty())
		engine_name = "eleeye_engine";
	if(theme_.empty())
		theme_ = "wood";
	board->set_themes(theme_);
	board->set_engine(engine_name);
	board->set_trace_color(color_.c_str());
}

MainWindow::~MainWindow()
{
	this->pop_colormap();
}

void MainWindow::on_size_change()
{
	int _wid = board->get_width();
	board->configure_board(_wid);
}

void MainWindow::change_play(bool player)
{
	if(player){
		p1_image->hide();
		p2_image->show();
	}
	else{
		p2_image->hide();
		p1_image->show();
	}
	
}

void MainWindow::save_conf()
{

	char buf[512];
	std::string homedir=Glib::get_user_config_dir();
	snprintf(buf, 512,"%s/gmchess/config",homedir.c_str());
	std::ofstream file(buf);
	std::string line;
	std::map<std::string,std::string>::iterator iter=GMConf.begin();
	for(;iter != GMConf.end(); ++iter)
	{
		line = iter->first + "\t=\t" + iter->second;
		file << line << std::endl;
	}
	file.close();

}

void MainWindow::init_conf()
{
	char buf[512];
	char book_dir[512];
	char file_dir[512];
	std::string homedir=Glib::get_user_config_dir();
	snprintf(book_dir, 512,"%s/gmchess/book",homedir.c_str());
	m_bookview->load_book_dir(book_dir);
	snprintf(buf,512,"%s/gmchess/config",homedir.c_str());

	std::ifstream file(buf);
	if(!file){
#ifdef __APPLE__
		mkdir(homedir.c_str(),S_IRUSR|S_IWUSR|S_IXUSR);
#endif
		char homepath[512];
		snprintf(homepath,512,"%s/gmchess/",homedir.c_str());
		mkdir(homepath,S_IRUSR|S_IWUSR|S_IXUSR);
		GMConf["usebook"]="1";
		GMConf["desktop_size"] = "1"; //0--small,1--big
		GMConf["engine_depth"] ="5";
		GMConf["themes"]="wood";
		GMConf["engine_name"] = "eleeye_engine";
		GMConf["line_color"] = "#198964";
		save_conf();

		snprintf(file_dir,512,"%s/gmchess/files",homedir.c_str());
		mkdir(file_dir,S_IRUSR|S_IWUSR|S_IXUSR);
		return;
	}
	
	std::string line;
	std::string name;
	std::string key;

	if(file){
		while(std::getline(file,line)){
			size_t pos= line.find_first_of("=");
			if(pos==std::string::npos)
				continue;
			name = line.substr(0,pos);
			key = line.substr(pos+1,std::string::npos);
			// 下面这2个把所有的空格都去掉了
			//key.erase(std::remove_if(key.begin(), key.end(), IsBlank()), key.end());
			//name.erase(std::remove_if(name.begin(), name.end(), IsBlank()), name.end());
			size_t pos1 = 0;
			size_t pos2 = 0;
			size_t len = 0;
			pos1 = name.find_first_not_of(" \t");
			pos2 = name.find_last_not_of(" \t");
			if (pos1 == std::string::npos || pos2 == std::string::npos)
				continue;
			len = pos2 - pos1 + 1;
			name = name.substr(pos1, len);

			pos1 = key.find_first_not_of(" \t");
			pos2 = key.find_last_not_of(" \t");
			if (pos1 == std::string::npos || pos2 == std::string::npos)
				continue;
			len = pos2 - pos1 + 1;
			key = key.substr(pos1, len);

			GMConf.insert(std::pair<std::string,std::string>(name,key));
		}
	}
	file.close();
}

void MainWindow::set_comment(const std::string& f_comment)
{
	text_comment->set_wrap_mode(Gtk::WRAP_WORD);
	Glib::RefPtr<Gtk::TextBuffer> buffer = text_comment->get_buffer();
	buffer->set_text(f_comment);

}

void MainWindow::show_textview_engine_log(const std::string& f_text)
{

	text_engine_log->set_wrap_mode(Gtk::WRAP_WORD);
	Glib::RefPtr<Gtk::TextBuffer> buffer_ = text_engine_log->get_buffer();
	Gtk::TextBuffer::iterator end_ = buffer_->end();

	Gdk::Rectangle rect_;
	text_engine_log->get_visible_rect(rect_);

	int y=-1;
	int height = -1;
	text_engine_log->get_line_yrange(end_,y,height);

	buffer_->place_cursor(buffer_->insert(end_,f_text));

	if(y<rect_.get_y() + rect_.get_height()+16)
		text_engine_log->scroll_to_mark(buffer_->get_insert(),0);
}
void MainWindow::textview_engine_log_clear()
{
	Glib::RefPtr<Gtk::TextBuffer> buffer_ = text_engine_log->get_buffer();
	Gtk::TextBuffer::iterator end_ , start_ ;
	buffer_->get_bounds(start_,end_);
	buffer_->erase(start_,end_);
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
void MainWindow::on_first_move()
{
	board->first_move();
	m_treeview.scroll_to_point(1,1);
}
void MainWindow::on_last_move()
{
	board->last_move();
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
	action = Gtk::Action::create("SaveAs",Gtk::Stock::SAVE,_("Save as"));
	action->set_tooltip(_("Save as a chess pgn file"));
	action_group->add(action,
			sigc::mem_fun(*this,&MainWindow::on_menu_save_file));
	action = Gtk::Action::create("SaveBoard", Gtk::Stock::SAVE, _("Save Board"));
	action->set_tooltip(_("Save borad to a png file"));
	action_group->add(action,
			sigc::mem_fun(*this, &MainWindow::on_menu_save_board_to_png));

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
	action_group->add(Gtk::Action::create("RevPlay",Gtk::Stock::MEDIA_PLAY,_("Change Side")),
			sigc::mem_fun(*this, &MainWindow::on_menu_rev_play));
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

void MainWindow::on_menu_save_board_to_png()
{
	board->save_board_to_file("/tmp/1.png");
}

void MainWindow::on_menu_save_file()
{

	Gtk::FileChooserDialog dlg(*this,
			_("Save File"), 
			Gtk::FILE_CHOOSER_ACTION_SAVE);
	dlg.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	dlg.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_OK);

	std::string filename ;
	if (Gtk::RESPONSE_OK == dlg.run()) {
		filename = dlg.get_filename();
	}
	if (filename.empty())
		return;


	std::ofstream  file;
	file.open(filename.c_str());
	if(!file){
		DLOG("open %s file error\n",filename.c_str());
		return ;
	}

	Gtk::TreeModel::Children children =  m_refTreeModel->children();
	Gtk::TreeModel::iterator iter ;
	for(iter = children.begin();iter!= children.end();iter++){

		file<<(*iter)[m_columns.step_bout] <<". "<<(*iter)[m_columns.step_line] ;
		iter++;
		if(iter!=children.end())
			file<<"  "<<(*iter)[m_columns.step_line]<<std::endl;
		else{
			file<<std::endl;
			break;
		}
	}

	file.close();

}

void MainWindow::auto_save_chess_file()
{
	std::string homedir=Glib::get_user_config_dir();
	homedir += "/gmchess/files/";

	char time1[200];
	char time2[200];
	time_t t;
	struct tm *tmp;

	t = time(NULL);
	tmp = localtime(&t);
	strftime(time1, sizeof(time1), "%Y-%m-%d-%H-%M-", tmp);
	strftime(time2, sizeof(time2), "%Y.%m.%d", tmp);
	std::string name = p1_name->get_text()+"-"+p2_name->get_text()+".pgn";
	std::string filename = homedir + std::string(time1) + name;

	std::ofstream  file;
	file.open(filename.c_str());
	if(!file){
		DLOG("open %s file error\n",filename.c_str());
		return ;
	}
	
	file<<"[Game \"Chese chess Play by GMChess\"]"<<std::endl;
	file<<"[Date \""<<std::string(time2)<<"\"]"<<std::endl;
	file<<"[Red \""<<p1_name->get_text()<<"\"]"<<std::endl;
	file<<"[Black \""<<p2_name->get_text()<<"\"]"<<std::endl;

	Gtk::TreeModel::Children children =  m_refTreeModel->children();
	Gtk::TreeModel::iterator iter ;
	for(iter = children.begin();iter!= children.end();iter++){

		file<<(*iter)[m_columns.step_bout] <<". "<<(*iter)[m_columns.step_line] ;
		iter++;
		if(iter!=children.end())
			file<<"  "<<(*iter)[m_columns.step_line]<<std::endl;
		else{
			file<<std::endl;
			break;
		}
	}

	file.close();


}


void MainWindow:: on_menu_open_file()
{
	Gtk::FileChooserDialog dlg(*this,
			_("Choose File"), 
			Gtk::FILE_CHOOSER_ACTION_OPEN);
	dlg.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	dlg.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

	Gtk::FileFilter filter_pgn;
	filter_pgn.set_name("PGN");
	filter_pgn.add_pattern("*.pgn");
	filter_pgn.add_pattern("*.PGN");
	dlg.add_filter(filter_pgn);
	/** 中游象棋*/
	Gtk::FileFilter filter_ccm;
	filter_ccm.set_name("CCM");
	filter_ccm.add_pattern("*.ccm");
	filter_ccm.add_pattern("*.CCM");
	dlg.add_filter(filter_ccm);
	/** QQ象棋*/
	Gtk::FileFilter filter_che;
	filter_che.set_name("CHE");
	filter_che.add_pattern("*.che");
	filter_che.add_pattern("*.CHE");
	dlg.add_filter(filter_che);
	/** 联众象棋*/
	Gtk::FileFilter filter_chn;
	filter_chn.set_name("CHN");
	filter_chn.add_pattern("*.chn");
	filter_chn.add_pattern("*.CHN");
	dlg.add_filter(filter_chn);

	/** 弈天象棋*/
	Gtk::FileFilter filter_mxq;
	filter_mxq.set_name("MXQ");
	filter_mxq.add_pattern("*.mxq");
	filter_mxq.add_pattern("*.MXQ");
	dlg.add_filter(filter_mxq);
	/** 象棋演播室*/
	Gtk::FileFilter filter_xqf;
	filter_xqf.set_name("XQF");
	filter_xqf.add_pattern("*.xqf");
	filter_xqf.add_pattern("*.XQF");
	dlg.add_filter(filter_xqf);
	/** 所有文件*/
	Gtk::FileFilter filter_any;
	filter_any.set_name(_("All Files"));
	filter_any.add_pattern("*");
	dlg.add_filter(filter_any);
	
	std::string filename ;
	if (Gtk::RESPONSE_OK == dlg.run()) {
		filename = dlg.get_filename();
	}
	if (filename.empty())
		return;
	open_file(filename);
}

void MainWindow::open_file(const std::string& filename)
{
	if(board->is_fight_to_robot()){
		Gtk::MessageDialog dialog(*this, _("AI Warn"), false,
                                  Gtk::MESSAGE_QUESTION,
                                  Gtk::BUTTONS_OK_CANCEL);
		Glib::ustring msg =_("Open book view will close the AI game,Are you sure?");
		dialog.set_secondary_text(msg);
		int result = dialog.run();
		switch (result) {
			case (Gtk::RESPONSE_OK): {
				board->free_game();
                	        break;
                	}

			case (Gtk::RESPONSE_CANCEL): {
				return;
                	        break;
                	}

			default: {
				return;
                	        break;
                	}
		}
	}

	/** 获取文件后先它将它转换成pgn文件才能打开*/
	/** get the file,and convert it to pgn file */
		int out;

		size_t pos = filename.find(".pgn");
		if(pos == std::string::npos){
			char cmd[1024];
			sprintf(cmd,"convert_pgn \"%s\" ",filename.c_str());
			if(system(cmd)<0){
				DLOG("convert pgn file error\n");
				return;
			}

			out = board->open_file("/tmp/gmchess.pgn");
		}
		else
			out = board->open_file( filename);
		if(out<0){
			DLOG("open file: %s error\n",filename.c_str());
			Gtk::MessageDialog dialog("Error",false,
					Gtk::MESSAGE_INFO);
			dialog.set_secondary_text(_("the file maybe not right format for chess"));
			dialog.run();
		}
		else{
			init_move_treeview();
			set_information();
			m_notebook->set_current_page(0);
		}


}

void MainWindow::info_window(const std::string& info)
{
			Gtk::MessageDialog dialog("Info",false,
					Gtk::MESSAGE_INFO);
			dialog.set_secondary_text(info);
			dialog.run();
}


void MainWindow::on_menu_war_to_ai()
{
	on_begin_game();
}

void MainWindow::on_menu_free_play()
{
	board->free_game();
	m_refTreeModel->clear();
	set_status();
}
void MainWindow::on_menu_rev_play()
{
	board->rev_game();

}

void MainWindow::on_menu_file_quit()
{
	if(board->get_status() == NETWORK_STATUS){
		board->send_to_socket("close");
	}
	Gtk::Main::quit();

}
bool MainWindow::on_delete_event(GdkEventAny* event)
{
	if(board->get_status() == NETWORK_STATUS){
		Gtk::MessageDialog dialog(*this, _("Warning"), false,
                                  Gtk::MESSAGE_QUESTION,
                                  Gtk::BUTTONS_OK_CANCEL);
		Glib::ustring msg =_("Are you  sure to close the network game?");
		dialog.set_secondary_text(msg);
		int result = dialog.run();
		switch (result) {
			case (Gtk::RESPONSE_OK): {
				board->send_to_socket("close");
                	        break;
                	}

			case (Gtk::RESPONSE_CANCEL): {
			     return true;
                	        break;
                	}

			default: {
			     return true;
                	        break;
                	}
		}
	}

	return Gtk::Window::on_delete_event(event);

}

void MainWindow::on_menu_view_preferences()
{
	if(NULL == confwindow){
		confwindow = new ConfWindow(this);
		confwindow->signal_quit().connect(sigc::mem_fun(*this, &MainWindow::on_conf_window_quit));
		confwindow->raise();
	}else
		confwindow->raise();
}

void MainWindow::on_conf_window_quit()
{
	save_conf();

	if(atoi(GMConf["desktop_size"].c_str()) == 1)
		board->set_board_size(BIG_BOARD);
	else
		board->set_board_size(SMALL_BOARD);
	board->set_themes(GMConf["themes"]);
}
void MainWindow::on_conf_window_close()
{
	if(NULL !=confwindow){
		delete confwindow;
		confwindow=NULL;
	}

}
void MainWindow::on_menu_help()
{
	printf("not realize yet\n");

}


void MainWindow::on_menu_about()
{
	static Gtk::AboutDialog*  about(0);
	if(about == 0){
		std::vector<Glib::ustring> authors;
		authors.push_back("lerosua@gmail.com ");
		authors.push_back("wind(xihels@gmail.com)");
		about = new Gtk::AboutDialog ;
		about->set_logo(ui_logo);
		if(GTKMM_MAJOR_VERSION==2 && GTKMM_MINOR_VERSION>=12)
			about->set_program_name("GMChess");
		about->set_version(version);
		about->set_website("http://lerosua.org");
		about->set_copyright("Copyright (c) 2009 - 2011 lerosua");
		about->set_comments(_("GMChess is chinese chess game write by gtkmm"));
		about->set_authors(authors);
		about->set_license (_("This program is licenced under GNU General Public Licence (GPL) version 2."));
		Glib::ustring transer = "zh_CN lerosua@gmail.com\nru Sadovnikov Dmitry <xbadcode@mail.ru>";
		about->set_translator_credits(transer);


	}
	about->run();
	delete about;
	about=0;

}

void MainWindow::add_step_line(int num,const Glib::ustring& f_line)
{
	Gtk::TreeModel::iterator iter = m_refTreeModel->append();
	(*iter)[m_columns.step_line] = f_line;
	(*iter)[m_columns.step_num] = num;
	(*iter)[m_columns.step_bout] = (int)((num+1)/2);
	if((num%2) ==0){
		(*iter)[m_columns.player] = _("Black");
	}
	else{
		(*iter)[m_columns.player] = _("Red");
	}

	Gtk::TreeModel::Path path(iter);
	m_treeview.scroll_to_row(path);
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

	if(board->is_fight_to_robot())
		return true;

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
	Gtk::Label* info   = 0;ui_xml->get_widget("info_label",info);
	
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
	set_status();

}


void MainWindow::set_status()
{
	int f_status = board->get_status();
	bool f_use=1;

	switch(f_status){
		case READ_STATUS:
			btn_next->set_sensitive(f_use);
			btn_prev->set_sensitive(f_use);
			btn_start->set_sensitive(f_use);
			btn_end->set_sensitive(f_use);

			btn_begin->set_sensitive(f_use);
			btn_lose->set_sensitive(1-f_use);
			btn_draw->set_sensitive(1-f_use);
			btn_rue->set_sensitive(1-f_use);
			break;
		case FIGHT_STATUS:
		case NETWORK_STATUS:
			btn_next->set_sensitive(1-f_use);
			btn_prev->set_sensitive(1-f_use);
			btn_start->set_sensitive(1-f_use);
			btn_end->set_sensitive(1-f_use);

			btn_begin->set_sensitive(1-f_use);
			btn_lose->set_sensitive(f_use);
			btn_draw->set_sensitive(f_use);
			btn_rue->set_sensitive(f_use);
			break;
		case FREE_STATUS:
			btn_next->set_sensitive(f_use);
			btn_prev->set_sensitive(f_use);
			btn_start->set_sensitive(f_use);
			btn_end->set_sensitive(f_use);

			btn_begin->set_sensitive(f_use);
			btn_lose->set_sensitive(1-f_use);
			btn_draw->set_sensitive(1-f_use);
			btn_rue->set_sensitive(1-f_use);
			break;
		default:
			break;
	};

}

void MainWindow::on_network_game(const std::string& me_name,const std::string& enemy_name,bool role_red_)
{
		p1_name->set_text(enemy_name);
		p2_name->set_text(me_name);
	if(!role_red_){
		board->rev_game();
	}
	
	m_refTreeModel->clear();
	board->start_network();
	set_status();
	btn_begin->set_sensitive(false);
		
	
}

void MainWindow::on_chanju_game()
{
	/** 已经在对战中，则询问是否开始新游戏*/
	/** ask if start new game */
	if(board->is_fight_to_robot()){

		Gtk::MessageDialog dialog(*this, _("new game"), false,
                                  Gtk::MESSAGE_QUESTION,
                                  Gtk::BUTTONS_OK_CANCEL);
		Glib::ustring msg =_("Will you  start a new game?");
		dialog.set_secondary_text(msg);
		int result = dialog.run();
		switch (result) {
			case (Gtk::RESPONSE_OK): {
				m_refTreeModel->clear();
				board->chanju_game();
                	        break;
                	}

			case (Gtk::RESPONSE_CANCEL): {
                	        break;
                	}

			default: {
                	        break;
                	}
		}
		return;

	}
	else if(board->is_network_game()){
		Gtk::MessageDialog dialog_info(*this, _("Information"), false);
		Glib::ustring msg =_("You are play with network game,Please over it first!");
		dialog_info.set_secondary_text(msg);
		dialog_info.run();
		return ;

	}
	m_refTreeModel->clear();
	board->start_robot(false);
	set_status();
	btn_begin->set_sensitive(false);
	//btn_chanjue->set_sensitive(false);


}

void MainWindow::on_begin_game()
{
	/** 已经在对战中，则询问是否开始新游戏*/
	/** ask if start new game */
	if(board->is_fight_to_robot()){

		Gtk::MessageDialog dialog(*this, _("new game"), false,
                                  Gtk::MESSAGE_QUESTION,
                                  Gtk::BUTTONS_OK_CANCEL);
		Glib::ustring msg =_("Will you  start a new game?");
		dialog.set_secondary_text(msg);
		int result = dialog.run();
		switch (result) {
			case (Gtk::RESPONSE_OK): {
				m_refTreeModel->clear();
				board->new_game();
                	        break;
                	}

			case (Gtk::RESPONSE_CANCEL): {
                	        break;
                	}

			default: {
                	        break;
                	}
		}
		return;

	}
	else if(board->is_network_game()){
		Gtk::MessageDialog dialog_info(*this, _("Information"), false);
		Glib::ustring msg =_("You are play with network game,Please over it first!");
		dialog_info.set_secondary_text(msg);
		dialog_info.run();
		return ;

	}
	m_refTreeModel->clear();
	board->start_robot();
	set_status();
	btn_begin->set_sensitive(false);
}

void MainWindow::on_lose_game()
{

    if(board->is_fight_to_robot()){
        Gtk::MessageDialog dialog(*this, _("be lose"), false,
                                  Gtk::MESSAGE_QUESTION,
                                  Gtk::BUTTONS_OK_CANCEL);
        Glib::ustring msg =_("Will you resign in this game?");
        dialog.set_secondary_text(msg);
        int result =dialog.run();
        switch (result) {
            case (Gtk::RESPONSE_OK): {
                //m_refTreeModel->clear();
                board->free_game();
                set_status();
                            break;
                    }

            case (Gtk::RESPONSE_CANCEL): {
                //board->free_game(false);
                //set_status();
                            break;
                    }

            default: {
                            break;
                    }
        }
        return;
    }
    else if(board->is_network_game()){

        Gtk::MessageDialog dialog(*this, _("be lose"), false,
                                  Gtk::MESSAGE_QUESTION,
                                  Gtk::BUTTONS_OK_CANCEL);
        Glib::ustring msg =_("Will you resign in this game?");
        dialog.set_secondary_text(msg);
        int result =dialog.run();
        switch (result) {
            case (Gtk::RESPONSE_OK): {
				board->send_to_socket("resign");
				Gtk::MessageDialog dialog_info(*this, _("Information"), false);
				Glib::ustring msg =_("You lose the game!");
				dialog_info.set_secondary_text(msg);
				dialog_info.run();
				board->free_game();
				set_status();
                            break;
				}

            case (Gtk::RESPONSE_CANCEL): {
                //board->free_game(false);
                //set_status();
                            break;
                    }

            default: {
                            break;
                    }
        }
    }
}

/** draw 是打平局面的意思*/
void MainWindow::on_draw_game()
{
    if(board->is_fight_to_robot()){
		board->draw_move();
    }
	else if(board->is_network_game()){
		if(board->is_human_player()){
			board->send_to_socket("draw");
		}else{
			Gtk::MessageDialog dialog(*this, _("Warning"), false);
			Glib::ustring msg =_("Only ask rue game when you are going,so wait a minute!");
			dialog.set_secondary_text(msg);
			dialog.run();
		}
	}

}

void MainWindow::on_rue_game()
{
	/** 如果是网络对战，则需要确认信息*/
	/** 设置成只有用户走时才能悔棋，一次撤销两步，即ai的一步，用户的一步，再次轮到用户走
	 * 这么做是为了防止ai在思考中撤销着法会产生冲突*/

	if(board->is_fight_to_robot()){
		if(board->is_human_player()){
			board->rue_move();
			board->rue_move();
		}else{
			Gtk::MessageDialog dialog(*this, _("Warning"), false);
			Glib::ustring msg =_("Only rue game when you are going,so wait a minute!");
			dialog.set_secondary_text(msg);
			dialog.run();
		}

	}
	else if(board->is_network_game()){
		if(board->is_human_player()){
			/** 发送了悔棋指令，此时界面不应该能操作，直到对方返回应答，yes/no */
			board->send_to_socket("rue");
		}else{
			Gtk::MessageDialog dialog(*this, _("Warning"), false);
			Glib::ustring msg =_("Only rue game when you are going,so wait a minute!");
			dialog.set_secondary_text(msg);
			dialog.run();
		}


	}

}
bool MainWindow::on_end_game(OVERSTATUS _over)
{
	Glib::ustring msg ;
	switch(_over){
		case ROBOT_WIN:
			//robot win
			msg=_("You are Lose! \n Are you want to start a new game?");
			CSound::play(SND_LOSS);
			break;
		case ROBOT_LOSE:
			//robot lose
			msg=_("Congratuations ! YOU WIN!\n start a new game click OK");
			CSound::play(SND_WIN);
			break;
		case ROBOT_DRAW:
			//robot want to draw
			msg=_("Draw Game!");
			break;
		case ROBOT_OVER_TIME:
			// robot overload time,lose
			msg = _("The Enemy overload the time, You Win!");
			CSound::play(SND_WIN);
			break;
		case HUMAN_OVER_TIME:
			msg = _("You overload the time, You Lose!");
			CSound::play(SND_LOSS);
		default:
			break;

	}
		Gtk::MessageDialog dialog_info(*this, _("Game End"), false);
		dialog_info.set_secondary_text(msg);
		dialog_info.run();

		if(board->is_network_game())
			auto_save_chess_file();
		board->free_game(false);
		set_status();
}

void MainWindow::set_red_war_time(const Glib::ustring& f_time,const Glib::ustring& c_time)
{

		p2_war_time->set_text(f_time);
		p2_step_time->set_text(c_time);


}
void MainWindow::set_black_war_time(const Glib::ustring& f_time,const Glib::ustring& c_time)
{

		p1_war_time->set_text(f_time);
		p1_step_time->set_text(c_time);
}
void MainWindow::watch_socket(int fd)
{
	board->watch_socket(fd);
	
}
void MainWindow::start_with(const std::string& param)
{
		if((param.find("network-game-red,"))!= std::string::npos){
			//start network game with red player
			std::string enemy_name,my_name;
			size_t pos_s,pos_e,pos_m;
			pos_s= param.find("enemy_name:");
			pos_e= param.find(",my_name:");
			pos_m= param.find_first_of("@");
			enemy_name = param.substr(pos_s+11,pos_m-pos_s-11);
			pos_m = param.find_last_of("@");
			my_name = param.substr(pos_e+9,pos_m-pos_e-9);
			
			on_network_game(enemy_name,my_name,true);
		}else if((param.find("network-game-black,")) != std::string::npos){
			//start network game with black player
			std::string enemy_name,my_name;
			size_t pos_s,pos_e,pos_m;
			pos_s= param.find("enemy_name:");
			pos_e= param.find(",my_name:");
			pos_m= param.find_first_of("@");
			enemy_name = param.substr(pos_s+11,pos_m-pos_s-11);
			pos_m = param.find_last_of("@");
			my_name = param.substr(pos_e+9,pos_m-pos_e-9);
			on_network_game(my_name,enemy_name,false);
		}
		else{
			if(!param.empty())
				open_file(param);
		}
}
