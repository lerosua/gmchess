/*
 * =====================================================================================
 *
 *       Filename:  BookView.h
 *
 *    Description:  棋书目录管理
 *
 * =====================================================================================
 */

#ifndef _BOOK_VIEW_H_
#define _BOOK_VIEW_H_

#include <gtk/gtk.h>
#include <string>

enum{ GROUP,MEMBER};

class MainWindow;

class BookView
{
	public:
		BookView(MainWindow* parent);
		~BookView();

		GtkWidget* widget() { return treeview; }
		bool get_list_iter(GtkTreeIter* iter, GtkTreeIter* parent, const std::string& groupname);
		bool add_group(GtkTreeIter* iter, const std::string& group);
		bool add_group(GtkTreeIter* iter, const std::string& g_parent, const std::string& group);
		void add_line(const std::string& groupname, const std::string& f_line, const std::string& f_path);
		gboolean on_button_press_event(GdkEventButton* ev);
		int load_book_dir(const char* path);

	private:
		static gboolean button_press_cb(GtkWidget* widget, GdkEventButton* event, gpointer data);

		GtkWidget* treeview;
		GtkTreeStore* tree_model;
		MainWindow* m_parent;
};

#endif
