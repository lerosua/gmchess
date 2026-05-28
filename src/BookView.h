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
		bool add_group(const std::string& group);
		bool add_group(const std::string& g_parent, const std::string& group);
		void add_line(const std::string& groupname, const std::string& f_line, const std::string& f_path);
		int load_book_dir(const char* path);

	private:
		static void setup_cb(GtkSignalListItemFactory* factory, GtkListItem* item, gpointer data);
		static void bind_cb(GtkSignalListItemFactory* factory, GtkListItem* item, gpointer data);
		static void activate_cb(GtkListView* list, guint position, gpointer data);
		int load_book_dir(const char* path, int depth);
		void append_row(const std::string& title, const std::string& path, int type, int depth);

		GtkWidget* treeview;
		GListStore* tree_model;
		GtkSingleSelection* selection_model;
		MainWindow* m_parent;
};

#endif
