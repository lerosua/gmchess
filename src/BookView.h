/*
 * =====================================================================================
 *
 *       Filename:  BookView.h
 *
 *    Description:  棋书目录管理
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

#ifndef _BOOK_VIEW_H_
#define _BOOK_VIEW_H_

#include <gtkmm.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <gtkmm/treeselection.h>
#include <functional>

enum{ GROUP,MEMBER};

class MainWindow;
class BookView: public Gtk::TreeView
{
	public:
		BookView(MainWindow* parent);
		~BookView();

		Gtk::TreeModel::iterator getListIter(Gtk::TreeModel::
				Children children, const std::string& groupname);
		/** 添加组*/
		/** add group */
		Gtk::TreeModel::iterator add_group(const Glib::ustring& group);
		Gtk::TreeModel::iterator add_group(const Glib::ustring& g_parent,const Glib::ustring& group);
		/** 添加行内容*/
		/** add line context */
		void add_line(const Glib::ustring& groupname,const Glib::ustring& f_line,const Glib::ustring& f_path);
		bool on_button_press_event(GdkEventButton *);
		int load_book_dir(const char* path);
	protected:
		class Columns:public Gtk::TreeModel::ColumnRecord
		{
			public:
				Columns()
				{
					add(title);
					add(path);
					add(type);
				}
				Gtk::TreeModelColumn <Glib::ustring> title;
				Gtk::TreeModelColumn <Glib::ustring> path;
				Gtk::TreeModelColumn <int> type;
		};
		Columns m_columns;
		Glib::RefPtr<Gtk::TreeStore>	m_refTreeModel;

		MainWindow* m_parent;
	private:
		struct Compare:public std::binary_function 
				      < Gtk::TreeModel::Row,
		const Glib::ustring,bool >{
			explicit Compare(const Columns& 
					column_):column(column_) {} 
			bool operator () (const Gtk::TreeRow& lhs,
					const Glib::ustring& var) const 
			{
				return lhs[column.title] == var; 
			}
			const Columns& column;
		};



};
#endif

