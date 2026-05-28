/*
 * =====================================================================================
 *
 *       Filename:  BookView.cpp
 *
 *    Description:
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
#include <cstring>

enum {
	COL_TITLE,
	COL_PATH,
	COL_TYPE,
	N_COLUMNS
};

void BookView::button_press_cb(GtkGestureClick* gesture, int n_press, double x, double y, gpointer data)
{
	guint button = gtk_gesture_single_get_current_button(GTK_GESTURE_SINGLE(gesture));
	static_cast<BookView*>(data)->on_button_press_event(x, y, button, n_press);
}

BookView::BookView(MainWindow* parent)
	: treeview(gtk_tree_view_new())
	, tree_model(gtk_tree_store_new(N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT))
	, m_parent(parent)
{
	gtk_widget_set_can_focus(treeview, TRUE);
	gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(tree_model));
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(treeview),
			-1, _("Book"), gtk_cell_renderer_text_new(), "text", COL_TITLE, NULL);
	GtkGesture* click = gtk_gesture_click_new();
	gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(click), 0);
	g_signal_connect(click, "pressed", G_CALLBACK(button_press_cb), this);
	gtk_widget_add_controller(treeview, GTK_EVENT_CONTROLLER(click));
	gtk_widget_show(treeview);
}

BookView::~BookView()
{
	g_object_unref(tree_model);
}

bool BookView::add_group(GtkTreeIter* iter, const std::string& group)
{
	gtk_tree_store_append(tree_model, iter, NULL);
	gtk_tree_store_set(tree_model, iter,
			COL_TITLE, group.c_str(),
			COL_PATH, "",
			COL_TYPE, GROUP,
			-1);
	return true;
}

bool BookView::add_group(GtkTreeIter* iter, const std::string& g_parent, const std::string& group)
{
	if(g_parent == "book")
		return add_group(iter, group);

	GtkTreeIter parent_iter;
	if(!get_list_iter(&parent_iter, NULL, g_parent))
		add_group(&parent_iter, g_parent);

	gtk_tree_store_append(tree_model, iter, &parent_iter);
	gtk_tree_store_set(tree_model, iter,
			COL_TITLE, group.c_str(),
			COL_PATH, "",
			COL_TYPE, GROUP,
			-1);
	return true;
}

void BookView::add_line(const std::string& groupname, const std::string& f_line, const std::string& f_path)
{
	GtkTreeIter group_iter;
	if(!get_list_iter(&group_iter, NULL, groupname)) {
		GtkTreeIter root_iter;
		bool found = false;
		if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tree_model), &root_iter)) {
			do {
				if(get_list_iter(&group_iter, &root_iter, groupname)) {
					found = true;
					break;
				}
			} while(gtk_tree_model_iter_next(GTK_TREE_MODEL(tree_model), &root_iter));
		}
		if(!found)
			return;
	}

	GtkTreeIter iter;
	gtk_tree_store_append(tree_model, &iter, &group_iter);
	gtk_tree_store_set(tree_model, &iter,
			COL_TITLE, f_line.c_str(),
			COL_PATH, f_path.c_str(),
			COL_TYPE, MEMBER,
			-1);
}

bool BookView::get_list_iter(GtkTreeIter* iter, GtkTreeIter* parent, const std::string& groupname)
{
	GtkTreeModel* model = GTK_TREE_MODEL(tree_model);
	if(!gtk_tree_model_iter_children(model, iter, parent))
		return false;

	do {
		gchar* title = NULL;
		gtk_tree_model_get(model, iter, COL_TITLE, &title, -1);
		const bool matched = title && groupname == title;
		g_free(title);
		if(matched)
			return true;
	} while(gtk_tree_model_iter_next(model, iter));

	return false;
}

gboolean BookView::on_button_press_event(double x, double y, guint button, int n_press)
{
	GtkTreePath* path = NULL;
	GtkTreeViewColumn* tvc = NULL;
	int cx, cy;
	if(!gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(treeview), (int)x, (int)y,
				&path, &tvc, &cx, &cy))
		return FALSE;

	GtkTreeIter iter;
	GtkTreeModel* model = GTK_TREE_MODEL(tree_model);
	if(!gtk_tree_model_get_iter(model, &iter, path)) {
		gtk_tree_path_free(path);
		return FALSE;
	}

	GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
	gtk_tree_selection_select_iter(selection, &iter);

	if (n_press >= 2 && button != 3) {
		gint type = GROUP;
		gchar* file = NULL;
		gtk_tree_model_get(model, &iter, COL_TYPE, &type, COL_PATH, &file, -1);
		if(GROUP != type){
			DLOG("open file %s \n", file ? file : "");
			if(file)
				m_parent->open_file(file);
		}
		else {
			if(gtk_tree_view_row_expanded(GTK_TREE_VIEW(treeview), path))
				gtk_tree_view_collapse_row(GTK_TREE_VIEW(treeview), path);
			else{
				gtk_tree_view_expand_row(GTK_TREE_VIEW(treeview), path, FALSE);
				gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(treeview), path, NULL, FALSE, 0, 0);
			}
		}
		g_free(file);
	}

	gtk_tree_path_free(path);
	return FALSE;
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

		stat(cPath, &pStat);
		if(S_ISDIR(pStat.st_mode)){
			GtkTreeIter iter;
			add_group(&iter, basename((char*)Path), basename(cPath));
			load_book_dir(cPath);
		}else{
			add_line(basename((char*)Path),node->d_name,cPath);
		}
	}
	closedir(dirp);

	return 0;
}
