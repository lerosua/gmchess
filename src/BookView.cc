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
	BOOK_GROUP,
	BOOK_MEMBER
};

static GObject* book_row_new(const std::string& title, const std::string& path, int type, int depth)
{
	GObject* row = G_OBJECT(g_object_new(G_TYPE_OBJECT, NULL));
	g_object_set_data_full(row, "title", g_strdup(title.c_str()), g_free);
	g_object_set_data_full(row, "path", g_strdup(path.c_str()), g_free);
	g_object_set_data(row, "type", GINT_TO_POINTER(type));
	g_object_set_data(row, "depth", GINT_TO_POINTER(depth));
	return row;
}

static const char* row_string(GObject* row, const char* key)
{
	const char* value = static_cast<const char*>(g_object_get_data(row, key));
	return value ? value : "";
}

static int row_int(GObject* row, const char* key)
{
	return GPOINTER_TO_INT(g_object_get_data(row, key));
}

void BookView::setup_cb(GtkSignalListItemFactory*, GtkListItem* item, gpointer)
{
	GtkWidget* label = gtk_label_new("");
	gtk_label_set_xalign(GTK_LABEL(label), 0.0);
	gtk_list_item_set_child(item, label);
}

void BookView::bind_cb(GtkSignalListItemFactory*, GtkListItem* item, gpointer)
{
	GObject* row = G_OBJECT(gtk_list_item_get_item(item));
	GtkWidget* label = gtk_list_item_get_child(item);
	const int depth = row_int(row, "depth");
	const int type = row_int(row, "type");
	std::string title(depth * 2, ' ');
	if(type == BOOK_GROUP)
		title += "[";
	title += row_string(row, "title");
	if(type == BOOK_GROUP)
		title += "]";
	gtk_label_set_text(GTK_LABEL(label), title.c_str());
	gtk_widget_set_margin_start(label, depth * 12);
}

void BookView::activate_cb(GtkListView*, guint position, gpointer data)
{
	BookView* self = static_cast<BookView*>(data);
	GObject* row = G_OBJECT(g_list_model_get_item(G_LIST_MODEL(self->tree_model), position));
	if(!row)
		return;
	if(row_int(row, "type") == BOOK_MEMBER) {
		const char* file = row_string(row, "path");
		DLOG("open file %s \n", file);
		if(file[0] != '\0')
			self->m_parent->open_file(file);
	}
	g_object_unref(row);
}

BookView::BookView(MainWindow* parent)
	: treeview(NULL)
	, tree_model(g_list_store_new(G_TYPE_OBJECT))
	, selection_model(NULL)
	, m_parent(parent)
{
	GtkListItemFactory* factory = gtk_signal_list_item_factory_new();
	g_signal_connect(factory, "setup", G_CALLBACK(setup_cb), this);
	g_signal_connect(factory, "bind", G_CALLBACK(bind_cb), this);
	selection_model = gtk_single_selection_new(G_LIST_MODEL(g_object_ref(tree_model)));
	treeview = gtk_list_view_new(GTK_SELECTION_MODEL(g_object_ref(selection_model)), factory);
	gtk_widget_set_can_focus(treeview, TRUE);
	g_signal_connect(treeview, "activate", G_CALLBACK(activate_cb), this);
	gtk_widget_set_visible(treeview, TRUE);
}

BookView::~BookView()
{
	g_object_unref(tree_model);
	g_object_unref(selection_model);
}

void BookView::append_row(const std::string& title, const std::string& path, int type, int depth)
{
	GObject* row = book_row_new(title, path, type, depth);
	g_list_store_append(tree_model, row);
	g_object_unref(row);
}

bool BookView::add_group(const std::string& group)
{
	append_row(group, "", BOOK_GROUP, 0);
	return true;
}

bool BookView::add_group(const std::string&, const std::string& group)
{
	append_row(group, "", BOOK_GROUP, 1);
	return true;
}

void BookView::add_line(const std::string& groupname, const std::string& f_line, const std::string& f_path)
{
	(void)groupname;
	append_row(f_line, f_path, BOOK_MEMBER, 1);
}

int BookView::load_book_dir(const char* Path)
{
	g_list_store_remove_all(tree_model);
	char* base = g_path_get_basename(Path);
	append_row(base ? base : "book", "", BOOK_GROUP, 0);
	g_free(base);
	return load_book_dir(Path, 1);
}

int BookView::load_book_dir(const char* Path, int depth)
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
			char* base = g_path_get_basename(cPath);
			append_row(base ? base : node->d_name, "", BOOK_GROUP, depth);
			g_free(base);
			load_book_dir(cPath, depth + 1);
		}else{
			append_row(node->d_name, cPath, BOOK_MEMBER, depth);
		}
	}
	closedir(dirp);

	return 0;
}
