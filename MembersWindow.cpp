#include <gtk/gtk.h>
#include "MembersWindow.h"
#include "julianday.h"
#include "financial.h"

const char* MembersWindow::headers[]={"Номер","ФИО"};


int MembersWindow::callback_orv_member(void* _store, int id, PARAMS_orv_member){
	GtkListStore *store = (GtkListStore *)_store;
	GtkTreeIter iter;
    gtk_list_store_append (store, &iter);
    gtk_list_store_set (store, &iter,
		col_num, id,
		col_fio, fio,
		col_pol_m, pol_m,
		col_pasp_ser, pasp_ser, 
		col_pasp_num, pasp_num, 
		col_pasp_dvyd, pasp_dvyd,
		col_pasp_kem_vyd, pasp_kem_vyd,
		col_addr_reg, addr_reg,
		col_contact, contact,
		col_fv_zayavka, fv_zayavka,
		col_comment, comment,			  
		-1);
	return 0;
}

int getJDfromCalendar(GtkCalendar *calendar){
	guint year, month, day;
	gtk_calendar_get_date(calendar,&year,&month,&day);
	return computeJD(day, month+1, year);	
}

void setJDinCalendar(GtkCalendar *calendar, int date){
	int year, month, day;
	computeYMD(date, &day, &month, &year);
	gtk_calendar_select_month (calendar, month-1, year);
	gtk_calendar_select_day (calendar, day);
}

void MembersWindow::bAddClicked(GtkButton *button, MembersWindow *mw){
	financial fv_zayavka;
	const char *fv_zayavka_text = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget(mw->xml, "e_fv_zayavka")));
	if(*fv_zayavka_text){
		if(str2financial(fv_zayavka_text, &fv_zayavka)<0){
			mw->showErrDialog("Некорректно введено число в поле 'Готов взять займ в сумме'");
			return;
		}
	}else
		fv_zayavka=0;
	mw->db->insert_orv_member(
		gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget(mw->xml, "e_fio"))), 
		gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(glade_xml_get_widget(mw->xml, "cb_pol_m"))),
		gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget(mw->xml, "e_pasp_ser"))), 
		gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget(mw->xml, "e_pasp_num"))), 
		gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(glade_xml_get_widget(mw->xml, "cb_is_pasp_dvyd") ))?
			getJDfromCalendar(GTK_CALENDAR(glade_xml_get_widget(mw->xml, "cal_pasp_dvyd")))
			:0, 
		gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget(mw->xml, "e_pasp_kem_vyd"))), 
		gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget(mw->xml, "e_addr_reg"))), 
		gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget(mw->xml, "e_contact"))), 
		0, fv_zayavka,
		gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget(mw->xml, "e_comment"))));
	mw->refresh();
}

void MembersWindow::bDelClicked(GtkButton *button, MembersWindow *mw){
	mw->db->delete_orv_member(mw->selected_id);
	mw->refresh();
}

void MembersWindow::bChangeClicked(GtkButton *button, MembersWindow *mw){
	financial fv_zayavka;
	if(str2financial(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget(mw->xml, "e_fv_zayavka"))), &fv_zayavka)<0){
		mw->showErrDialog("Некорректно введено число в поле 'Готов взять займ в сумме'");
		return;
	}
	mw->db->update_orv_member(mw->selected_id, 
		gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget(mw->xml, "e_fio"))), 
		gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(glade_xml_get_widget(mw->xml, "cb_pol_m"))),
		gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget(mw->xml, "e_pasp_ser"))), 
		gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget(mw->xml, "e_pasp_num"))), 
		gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(glade_xml_get_widget(mw->xml, "cb_is_pasp_dvyd") ))?
			getJDfromCalendar(GTK_CALENDAR(glade_xml_get_widget(mw->xml, "cal_pasp_dvyd")))
			:0, 
		gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget(mw->xml, "e_pasp_kem_vyd"))), 
		gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget(mw->xml, "e_addr_reg"))), 
		gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget(mw->xml, "e_contact"))), 
		0, fv_zayavka,
		gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget(mw->xml, "e_comment")))
	);
	mw->refresh();
}

void MembersWindow::refresh(){
	GtkWidget*tvMembers = glade_xml_get_widget(xml, "tvMembers");	
	GtkListStore *store = gtk_list_store_new (num_cols,
				  G_TYPE_UINT,
				  G_TYPE_STRING,	
				  G_TYPE_BOOLEAN,			  
				  G_TYPE_STRING,
				  G_TYPE_STRING,
				  G_TYPE_UINT,
				  G_TYPE_STRING,
				  G_TYPE_STRING,
				  G_TYPE_STRING,
				  G_TYPE_INT64,
				  G_TYPE_STRING				  
				  );	
	db->select_orv_member(0, callback_orv_member, store);	
	gtk_tree_view_set_model(GTK_TREE_VIEW(tvMembers), GTK_TREE_MODEL (store));
	g_object_unref (G_OBJECT(store));	
}

#define SET_AND_FREE(name) gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget(mw->xml, "e_"#name)), (name)?name:""); g_free(name);

void MembersWindow::selection_changed (GtkTreeSelection *selection, MembersWindow *mw)
{
	gtk_widget_set_sensitive ( glade_xml_get_widget(mw->xml, "bChange"), 1);
	gtk_widget_set_sensitive ( glade_xml_get_widget(mw->xml, "bDel"), 1);
	GtkTreeModel *model;
	GtkTreeIter iter;
	if (gtk_tree_selection_get_selected (selection, &model, &iter)){
		char* fio;
		char* pasp_ser;
		char* pasp_num; 
		int pasp_dvyd;
		int pol_m;
		char* pasp_kem_vyd;
		char* addr_reg;
		char* contact;
		financial fv_zayavka;
		char* comment;		
		gtk_tree_model_get (model, &iter,
				col_num, &(mw->selected_id),
				col_pol_m, &pol_m,
				col_fio, &fio,
				col_pasp_ser, &pasp_ser, 
				col_pasp_num, &pasp_num, 
				col_pasp_dvyd, &pasp_dvyd,
				col_pasp_kem_vyd, &pasp_kem_vyd,
				col_addr_reg, &addr_reg,
				col_contact, &contact,
				col_fv_zayavka, &fv_zayavka,
				col_comment, &comment,			
				-1);
		SET_AND_FREE(fio);
		SET_AND_FREE(pasp_ser);
		SET_AND_FREE(pasp_num);
		SET_AND_FREE(pasp_kem_vyd);
		SET_AND_FREE(addr_reg);
		SET_AND_FREE(contact);
		SET_AND_FREE(comment);
		if(pasp_dvyd)
			setJDinCalendar(GTK_CALENDAR(glade_xml_get_widget(mw->xml, "cal_pasp_dvyd")), pasp_dvyd);					
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(glade_xml_get_widget(mw->xml, "cb_is_pasp_dvyd") ),pasp_dvyd);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(glade_xml_get_widget(mw->xml, "cb_pol_m") ),pol_m);		
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget(mw->xml, "e_fv_zayavka")), financial2str(fv_zayavka).c_str());
	}else{
		gtk_widget_set_sensitive ( glade_xml_get_widget(mw->xml, "bChange"), 0);
		gtk_widget_set_sensitive ( glade_xml_get_widget(mw->xml, "bDel"), 0);
		mw->selected_id = 0;
		const char* entries[]={"e_fio","e_pasp_ser","e_pasp_num","e_pasp_kem_vyd","e_addr_reg","e_contact","e_comment","e_fv_zayavka",0};
		for(const char**e=entries;*e;e++)
			gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget(mw->xml, *e)), "");
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(glade_xml_get_widget(mw->xml, "cb_is_pasp_dvyd") ),0);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(glade_xml_get_widget(mw->xml, "cb_pol_m") ),0);	
	}
}


void cb_is_pasp_dvyd_toggled (GtkToggleButton *togglebutton, GtkWidget *calendar){
	gtk_widget_set_sensitive ( calendar, gtk_toggle_button_get_active (togglebutton));	
}
                                                        
MembersWindow::MembersWindow(DB* _db):db(_db),selected_id(0),Base("membersWindow"){		
	if (!xml) {
		g_warning("Failed to create the interface: membersWindow");
		return;
	}


	glade_xml_signal_autoconnect(xml);

	GtkWidget* w = glade_xml_get_widget(xml, "membersWindow");
	gtk_window_maximize(GTK_WINDOW(w));
	GtkTreeView *tvMembers = GTK_TREE_VIEW(glade_xml_get_widget(xml, "tvMembers"));
	for(int i=0;i<sizeof(headers)/sizeof(*headers)/*num_cols*/;i++){		
		GtkCellRenderer *renderer = gtk_cell_renderer_text_new ();
		GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes (headers[i],
								 renderer,
								 "text", i,
								 NULL);
		gtk_tree_view_column_set_resizable(column, true);
		gtk_tree_view_column_set_min_width(column, 10);
		gtk_tree_view_column_set_sort_column_id (column, i);
		gtk_tree_view_append_column (tvMembers, column);
	}
	refresh();
	GtkTreeSelection *selection = gtk_tree_view_get_selection (tvMembers);
	gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);
	g_signal_connect (selection,"changed",		G_CALLBACK (selection_changed),		this);	
	g_signal_connect (glade_xml_get_widget(xml, "bAdd"), "clicked",  G_CALLBACK (bAddClicked), this);
	g_signal_connect (glade_xml_get_widget(xml, "bDel"), "clicked",  G_CALLBACK (bDelClicked), this);
	g_signal_connect (glade_xml_get_widget(xml, "bChange"), "clicked",  G_CALLBACK (bChangeClicked), this);
	g_signal_connect (glade_xml_get_widget(xml, "cb_is_pasp_dvyd"), "toggled",  G_CALLBACK (cb_is_pasp_dvyd_toggled), glade_xml_get_widget(xml, "cal_pasp_dvyd"));
	gtk_widget_show_all(w);
}

MembersWindow::~MembersWindow(){}
