#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <gtk/gtk.h>
#include <glade/glade.h>
#include "base.h"
#include "db.h"
class MainWindow: public Base{
	enum
	{
	  main_col_num,
	  main_col_signed,
	  main_col_depositor_id,
	  main_col_depositor,
	  main_col_user,
	  main_col_poruch,
	  main_col_sum,
	  main_col_rem,
	  main_col_end_date,
	  main_col_closed,
	  main_num_cols
	};	
	GtkTreeModel *main_model;
	GtkWidget* tvAgrList;
	
	static const char* main_headers[];
	static void callback_fv_agr(void* mw, int id, PARAMS_fv_agr_select);
	static void bCreateClicked(GtkButton *button, MainWindow *mw);
	static void bCreateStandardClicked(GtkButton *button, MainWindow *mw);
	static void bDeleteClicked(GtkButton *button, MainWindow *mw);
	static void bMembersClicked(GtkButton *button, MainWindow *mw);
	static void tvAgrListRowActivated(GtkTreeView        *treeview,
                       GtkTreePath        *path,
                       GtkTreeViewColumn  *col,
                       MainWindow *mw);
    static void rbToggled(GtkToggleButton *togglebutton, MainWindow *mw);

	DB* db;
public:
	void refresh();
	MainWindow(DB* _db);
	~MainWindow(); //Закрытие БД
};



#endif 
