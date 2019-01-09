#include "MainWindow.h"
#include "AgrWindow.h"
#include "MembersWindow.h"
#include "julianday.h"
#include "financial.h"
#include "PaymentProcessing.h"
#include <glade/glade.h>
#include <sstream>

const char* MainWindow::main_headers[]={"Номер","Дата подписания","ID Займодавца" /*Не отображается*/,"Займодавец","Пользователи","Поручители","Выдано","Долг на сегодня","Запланированная дата окончания","Дата закрытия"};


int MainWindow::callback_fv_agr(void* mw, int id, PARAMS_fv_agr_select){
  MainWindow *mainW = (MainWindow *)mw;
  GtkListStore *store = (GtkListStore *)(mainW->main_model);
  GtkTreeIter iter;
  gtk_list_store_append (store, &iter);
  char end_date_str[2+1+2+1+4 + 1];
  JD2str(end_date, end_date_str);
  char signed_date_str[2+1+2+1+4 + 1];
  JD2str(signed_date, signed_date_str);
  char closed_date_str[2+1+2+1+4 + 1];
  JD2str(closed, closed_date_str);
  std::ostringstream ss;
  ss << depositor<<" ("<<depositor_id<<")";
	
  PaymentProcessing pp(id, mainW->db);
  financial rem = pp.getAllDolg(JDnow());
	  
  gtk_list_store_set (store, &iter,
		  main_col_num, id,
		  main_col_signed, signed_date_str,
		  main_col_depositor_id, depositor_id,
		  main_col_depositor, ss.str().c_str(),
		  main_col_user, users,
		  main_col_poruch, poruch,
		  main_col_sum, financial2str(sum).c_str(),
		  main_col_rem, financial2str(rem).c_str(),
		  main_col_end_date, end_date_str,
		  main_col_closed, closed_date_str,
		  -1);
}

void MainWindow::refresh(){
	tvAgrList = glade_xml_get_widget(xml, "tvAgrList");	
    GtkListStore *store = gtk_list_store_new (main_num_cols,
				  G_TYPE_UINT,
				  G_TYPE_STRING,
				  G_TYPE_UINT,
				  G_TYPE_STRING,
				  G_TYPE_STRING,
				  G_TYPE_STRING,
				  G_TYPE_STRING,
				  G_TYPE_STRING,
				  G_TYPE_STRING,
				  G_TYPE_STRING);
	const char* filter="";
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(xml, "rbCurrent"))))
		filter=" where closed is null";
	else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(xml, "rbClosed"))))
		filter=" where closed is not null";
	main_model=GTK_TREE_MODEL (store);
    db->select_fv_agr(filter, callback_fv_agr, this);

	gtk_tree_view_set_model(GTK_TREE_VIEW(tvAgrList), main_model);
	g_object_unref (G_OBJECT(store));	
}

void MainWindow::bCreateClicked(GtkButton *button, MainWindow *mw){
	AgrWindow* agrW = new AgrWindow(mw, mw->db, 0, 0, false);
}

void MainWindow::bCreateStandardClicked(GtkButton *button, MainWindow *mw){
	AgrWindow* agrW = new AgrWindow(mw, mw->db, 0, 0, false, true);
}

void MainWindow::bMembersClicked(GtkButton *button, MainWindow *mw){
	MembersWindow *membW = new MembersWindow(mw->db);
}

void MainWindow::tvAgrListRowActivated(GtkTreeView        *treeview,
                       GtkTreePath        *path,
                       GtkTreeViewColumn  *col,
                       MainWindow *mw){
	
	GtkTreeIter iter;

    if (gtk_tree_model_get_iter(mw->main_model, &iter, path))
    {
		int agr_id;
		int depositor_id;
		char* closed;
		gtk_tree_model_get (mw->main_model, &iter,
			main_col_num,&agr_id,
			main_col_depositor_id, &depositor_id,
			main_col_closed, &closed,
			-1);
		AgrWindow* agrW = new AgrWindow(mw, mw->db, agr_id, depositor_id, (*closed!=0));	
		g_free(closed);
	}	
}
void MainWindow::bDeleteClicked(GtkButton *button, MainWindow *mw){		
	
	GtkTreeIter iter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(mw->tvAgrList));
	GtkTreeModel *model;
	if (gtk_tree_selection_get_selected (selection, &model, &iter)){
		int agr_id;
		gtk_tree_model_get (model, &iter,
				main_col_num, &agr_id,
				-1);
		//~ printf("%i\n", agr_id);
		//Окно с вопросом о подтверждении удаления - из ГИСа
		if(mw->showAcceptDialog("Вы уверены, что хотите удалить договор?")== GTK_RESPONSE_YES)
			mw->db->delete_fv_agr(agr_id);
		mw->refresh();	
	}else mw->showInfoDialog("Не выбран договор для удаления");
		
}

void MainWindow::rbToggled(GtkToggleButton *togglebutton, MainWindow *mw){
	mw->refresh();	
}
	
MainWindow::MainWindow(DB* _db):db(_db), Base("mainWindow"){

	if (!xml) {
		g_warning("Failed to create the interface: mainWindow");
		return;
	}


	glade_xml_signal_autoconnect(xml);

	GtkWidget* mainWindow = glade_xml_get_widget(xml, "mainWindow");
	gtk_window_maximize(GTK_WINDOW(mainWindow));
	
	
	refresh();
	
	for(int i=0;i<main_num_cols;i++){		
		if(i==main_col_depositor_id)
			continue;
		GtkCellRenderer *renderer = gtk_cell_renderer_text_new ();
		GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes (main_headers[i],
								 renderer,
								 "text", i,
								 NULL);
		gtk_tree_view_column_set_resizable(column, true);
		gtk_tree_view_column_set_min_width(column, 10);								 
		gtk_tree_view_column_set_sort_column_id (column, i);
		gtk_tree_view_append_column (GTK_TREE_VIEW(tvAgrList), column);
	}
	g_signal_connect(tvAgrList, "row-activated", G_CALLBACK (tvAgrListRowActivated), this);
	g_signal_connect (glade_xml_get_widget(xml, "bCreate"), "clicked",  G_CALLBACK (bCreateClicked), this);
	g_signal_connect (glade_xml_get_widget(xml, "bCreateStandard"), "clicked",  G_CALLBACK (bCreateStandardClicked), this);
	g_signal_connect (glade_xml_get_widget(xml, "bDelete"), "clicked",  G_CALLBACK (bDeleteClicked), this);
	g_signal_connect (glade_xml_get_widget(xml, "bMembers"), "clicked",  G_CALLBACK (bMembersClicked), this);
	g_signal_connect (glade_xml_get_widget(xml, "bExport2Site"), "clicked",  G_CALLBACK (showNotImplementedDialog), this);
	g_signal_connect (glade_xml_get_widget(xml, "bExport2Mobile"), "clicked",  G_CALLBACK (showNotImplementedDialog), this);
	g_signal_connect (glade_xml_get_widget(xml, "rbClosed"), "toggled",  G_CALLBACK (rbToggled), this);	//rbToggled вызывается дважды?
	g_signal_connect (glade_xml_get_widget(xml, "rbAll"), "toggled",  G_CALLBACK (rbToggled), this);
	g_signal_connect (glade_xml_get_widget(xml, "rbCurrent"), "toggled",  G_CALLBACK (rbToggled), this);
	gtk_widget_show_all(mainWindow);		
}

MainWindow::~MainWindow(){}; //Закрытие БД



