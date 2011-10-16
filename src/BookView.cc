/*
 * =====================================================================================
 *
 *       Filename:  BookView.cpp
 *
 *    Description:  
 *
 *        Version:  0.11
 *        Created:  2009年03月31日 13时55分18秒
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  lerosua@gmail.com
 *        Company:  cyclone
 *
 * =====================================================================================
 */


#include "MainWindow.h"
#include "BookView.h"
#include <glib/gi18n.h>
#include <dirent.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/errno.h>


BookView::BookView(MainWindow* parent):m_parent(parent)
{

	this->set_flags(Gtk::CAN_FOCUS);
	this->set_rules_hint(false);

	m_refTreeModel = Gtk::TreeStore::create(m_columns);
	this->set_model( m_refTreeModel);
	this->append_column(_("Book"), m_columns.title);


	this->show();
}


BookView::~BookView()
{
}

Gtk::TreeModel::iterator BookView::add_group(const Glib::ustring& group)
{
	Gtk::TreeModel::iterator iter = m_refTreeModel->append();
	(*iter)[m_columns.title]=group;
	(*iter)[m_columns.type] = GROUP;

	return iter;
}

Gtk::TreeModel::iterator BookView::add_group(const Glib::ustring& g_parent,const Glib::ustring& group)
{

	if(g_parent=="book"){
		return add_group(group);
	}
	Gtk::TreeModel::Children children = m_refTreeModel->children();
	Gtk::TreeModel::iterator listiter;
	listiter = getListIter(children,g_parent);
	if(listiter == children.end())
		listiter = add_group(g_parent);

	Gtk::TreeModel::iterator iter = m_refTreeModel->append(listiter->children());
	(*iter)[m_columns.title]=group;
	(*iter)[m_columns.type] = GROUP;

	return iter;
}



void BookView::add_line(const Glib::ustring& groupname,const Glib::ustring& f_line,const Glib::ustring& f_path)
{
	Gtk::TreeModel::Children children = m_refTreeModel->children();
	Gtk::TreeModel::iterator listiter;
	listiter = getListIter(children,groupname);
	if(listiter == children.end()){
		//listiter = add_group(groupname);

		Gtk::TreeModel::iterator t_iter=children.begin();
		Gtk::TreeModel::Children grandson= (*t_iter)->children();
		do{
			listiter = getListIter(grandson,groupname);
			if(listiter != grandson.end())
				break;
			t_iter++;
			grandson= (*t_iter)->children();
		}while(t_iter!=children.end());

		if(listiter == grandson.end())
			return;
	}

	Gtk::TreeModel::iterator iter = m_refTreeModel->append(listiter->children());
	(*iter)[m_columns.title] = f_line;
	(*iter)[m_columns.type] = MEMBER;
	(*iter)[m_columns.path]= f_path;
	


}

Gtk::TreeModel::iterator BookView::getListIter(Gtk::TreeModel::
		Children children, const std::string& groupname)
{
	return find_if(children.begin(),
			children.end(),
			bind2nd(Compare(m_columns),groupname));
}


bool BookView::on_button_press_event(GdkEventButton * ev)
{
	bool result = Gtk::TreeView::on_button_press_event(ev);

	Glib::RefPtr < Gtk::TreeSelection > selection =
		this->get_selection();
	Gtk::TreeModel::iterator iter = selection->get_selected();
	if (!selection->count_selected_rows())
		return result;

	Gtk::TreeModel::Path path(iter);
	Gtk::TreeViewColumn * tvc;
	int cx, cy;
	/** get_path_at_pos() 是为确认鼠标是否在选择行上点击的*/
	if (!this->
			get_path_at_pos((int) ev->x, (int) ev->y, path, tvc, cx, cy))
		return false;

	if ((ev->type == GDK_2BUTTON_PRESS ||
				ev->type == GDK_3BUTTON_PRESS) && ev->button != 3) {
		if(GROUP != (*iter)[m_columns.type]){
			Glib::ustring t_file = (*iter)[m_columns.path];
			DLOG("open file %s \n",t_file.c_str());
			m_parent->open_file(t_file.c_str());
		}
		else {
			if(this->row_expanded(path))
				this->collapse_row(path);
			else{
				this->expand_row(path,false);
				this->scroll_to_row(path);
			}
		}
	} else if ((ev->type == GDK_BUTTON_PRESS)
			&& (ev->button == 3)) {
		/*
		if(GROUP_CHANNEL == (*iter)[m_columns.type])
			return false;
		Gtk::Menu* pop_menu = 
			parent->get_channels_pop_menu();
		if (pop_menu)
			pop_menu->popup(ev->button, ev->time);
		return true;
		*/
	}
	return false;
}


int BookView::load_book_dir(const char* Path)
{
 
	DIR *dirp;
	struct dirent * node;
	char cPath[1024];
	struct stat pStat;

	if(mkdir(Path, 0755)==-1 && errno != EEXIST) {
		printf("Unable to create folder %s\n", Path);
		return -1;
	}


	if((dirp= opendir(Path))==NULL){
		printf("Could not read folder %s\n",Path);
		return -1;
	}

	while(( node = readdir(dirp))!= NULL){
		if((strcmp(node->d_name, ".") ==0)
				|| (strcmp(node->d_name, "..") == 0))
			continue;

		strcpy(cPath,Path);
		strcat(cPath,"/");
		strcat(cPath,node->d_name);

		//stat(node->d_name, &pStat);
		stat(cPath, &pStat);
		if(S_ISDIR(pStat.st_mode)){
			//printf(" dir = %s \n",cPath);
			/** 是目录，继续打开读*/
			/**  it is a directory,continue read */
			add_group(basename((char*)Path),basename(cPath));
			load_book_dir(cPath);

		}else{
			//printf(" add file = %s\n",basename(node->d_name));
			add_line(basename((char*)Path),node->d_name,cPath);
		}
	}
	closedir(dirp);

	return 0;
}
