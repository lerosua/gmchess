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
#include <sys/stat.h>


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

	return iter;
}


void BookView::add_line(const Glib::ustring& groupname,const Glib::ustring& line)
{
	Gtk::TreeModel::Children children = m_refTreeModel->children();
	Gtk::TreeModel::iterator listiter;
	listiter = getListIter(children,groupname);
	if(listiter == children.end())
		listiter = add_group(groupname);

	Gtk::TreeModel::iterator iter = m_refTreeModel->append(listiter->children());
	(*iter)[m_columns.title] = line;
	


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
#if 0
		if(GROUP_CHANNEL != (*iter)[m_columns.type]){
			play_selection_iter(iter);
		}
		else {
			if(this->row_expanded(path))
				this->collapse_row(path);
			else{
				this->expand_row(path,false);
				this->scroll_to_row(path);
			}
		}
#endif
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


int BookView::load_book_dir(char* Path)
{
 
	DIR *dirp;
	struct dirent * node;
	char cPath[256];
	struct stat pStat;

	if((dirp= opendir(Path))==NULL){
		printf("The dir %s is not exit\n",Path);
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
			load_book_dir(cPath);

		}else{
			printf(" add file = %s\n",basename(node->d_name));
			add_line(basename(Path),node->d_name);
		}
	}
	closedir(dirp);

	return 0;
}
