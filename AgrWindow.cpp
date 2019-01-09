#include <gtk/gtk.h>
#include <glade/glade.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "AgrWindow.h"
#include "PaymentProcessing.h"
#include "julianday.h"
#include "agr2odt.h"
#include "odflib/num2word.h"
#include <glib-object.h>

using namespace std;

const char* AgrWindow::schedule_headers[]={"Номер","Дата начала","Причина создания","Сумма операции","Комментарий"};
const char* AgrWindow::schedule_state_headers[]={"Дата","Основной долг","Проценты за ОД","Платеж по графику","Сумма платежа", "Просроченный долг","Проценты за ПД","Погашение ПД","Комментарий","Номер платежа"};
const char* reasonTypes[]={"Выдача","Платеж в счет следующего"};


void AgrWindow::schedule_refresh(){
    GtkListStore *store = gtk_list_store_new (schedule_num_cols,
				  G_TYPE_UINT,
				  G_TYPE_STRING,
				  G_TYPE_STRING,
				  G_TYPE_STRING,
				  G_TYPE_STRING);
    db->select_fv_schedules(agr_id, callback_fv_schedules, store);
	gtk_tree_view_set_model(GTK_TREE_VIEW(tvSchedule), schedule_model=GTK_TREE_MODEL (store));
	g_object_unref (G_OBJECT(store));	
 {
	GtkTreeIter iter;
	if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter)){
		GtkTreeIter iter_last = iter;
		while(gtk_tree_model_iter_next (GTK_TREE_MODEL(store), &iter)){
			iter_last = iter;
		}
		GtkTreeSelection *selection	= gtk_tree_view_get_selection (GTK_TREE_VIEW(tvSchedule));
		gtk_tree_selection_select_iter(selection, &iter_last);
		tvSchedule_selection_changed (selection, this);
	}
 }
 {
	GtkTreeIter iter;
	if(gtk_tree_model_get_iter_first(schedule_model, &iter)){ //Если уже создан хотя бы 1 график
		gtk_widget_hide(expanderVydacha);
		gtk_widget_show(expanderPogashenie);		
		gtk_expander_set_expanded(GTK_EXPANDER(expanderPogashenie), true);
		//~ gtk_paned_set_position(GTK_PANED(glade_xml_get_widget(xml, "vpanedSched")), 100);
	}else{
		gtk_widget_hide(expanderPogashenie);
		gtk_widget_show(expanderVydacha);
		gtk_expander_set_expanded(GTK_EXPANDER(expanderVydacha), true);		
	}	
 }		
 updateCalendarEntryAllDolg(calAllDolg, this);
}

static void set_cb_data(const char* param_name, int value, GtkTreeModel* store, GtkComboBox *w){
	g_object_set_data(G_OBJECT(w), param_name, (gpointer)value);
	GtkTreeIter iter;
	if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter))
		do{
			int id;
			gtk_tree_model_get(GTK_TREE_MODEL(store), &iter,0,&id,-1);
			if(id==value){
				gtk_combo_box_set_active_iter(GTK_COMBO_BOX(w), &iter);
				break;
			}
		}while(gtk_tree_model_iter_next (GTK_TREE_MODEL(store), &iter));	
}

static int get_cb_data(GtkComboBox *w){
	GtkTreeIter iter;
	if(gtk_combo_box_get_active_iter(w, &iter)){
		int id;
		gtk_tree_model_get(gtk_combo_box_get_model(w), &iter,0,&id,-1);
		return id;
	}
	return 0;
}

static void updateCalendarEntry(GtkCalendar *calendar, GtkEntry *e){
	guint year, month, day;
	gtk_calendar_get_date(calendar,&year,&month,&day);
	char buf[2+1+2+1+4 + 1];
	snprintf(buf, sizeof(buf), "%02i.%02i.%04i", day, month+1, year);
	gtk_entry_set_text(e, buf);
}


static void saveDate2 (GtkCalendar *calendar, GtkEntry* e){
   updateCalendarEntry(calendar, e);
   gtk_widget_hide(GTK_WIDGET(calendar));														
}

void AgrWindow::updateCalendarEntryAllDolg(GtkCalendar *calendar, AgrWindow* aw){
	guint year, month, day;
	gtk_calendar_get_date(calendar,&year,&month,&day);
	char buf[2+1+2+1+4 + 1];
	snprintf(buf, sizeof(buf), "%02i.%02i.%04i", day, month+1, year);
	gtk_entry_set_text(GTK_ENTRY(aw->eAllDolg), buf);
	if(aw->agr_id == 0){
		gtk_entry_set_text(GTK_ENTRY(aw->elAllDolg), "");
		return;
	}
	int date = computeJD(day, month+1, year);
	
	
	PaymentProcessing pp(aw->agr_id, aw->db);
	string proc;
	financial rem = pp.getAllDolg(date, &proc);
	gtk_entry_set_text(GTK_ENTRY(aw->elAllDolg), (rem)?((financial2str(rem)+" Проценты = "+proc).c_str()):"0");	
}


void AgrWindow::saveDate2AllDolg (GtkCalendar *calendar, AgrWindow* aw){
   updateCalendarEntry(calendar, GTK_ENTRY(aw->eAllDolg));
   gtk_widget_hide(GTK_WIDGET(calendar));														
}

static void closeDateSelected (GtkCalendar *calendar, GtkDialog* d){
	gtk_dialog_response(d, GTK_RESPONSE_OK);
}



static gboolean showCalendarAndFocusOnIt (GtkWidget *widget, GdkEvent  *event, GtkWidget*   c){	
	gtk_widget_show(c); 
	gtk_widget_grab_focus(c);
	return true;
}

class HBoxToVBox{
	GtkWidget *vbox;
	GtkWidget *hbox;
public:	
	HBoxToVBox(GladeXML *xml, const char* vboxName){
		vbox = glade_xml_get_widget (xml, vboxName);
		hbox = gtk_hbox_new(0,3/*spacing*/);
	}
	void packStart(GtkWidget* w){
		gtk_box_pack_start(GTK_BOX(hbox), w, 1,1,0);
	}
	~HBoxToVBox(){
		GtkWidget *e = gtk_button_new_with_label(" - ");
			g_signal_connect_swapped (G_OBJECT (e), "clicked", G_CALLBACK (gtk_widget_hide), hbox);
		gtk_box_pack_start(GTK_BOX(hbox), e, 0,0,0);
		gtk_box_pack_start(GTK_BOX(vbox), hbox, 0,0,0);
		gtk_widget_show_all(hbox);	
	}
};



GtkWidget * AgrWindow::addRate(int date, double value, int type_id, bool is_old){
	GtkWidget * c;
	{
		HBoxToVBox packer(xml, ((type_id==1)?"vboxMainRate":"vboxPrRate"));		
		{
			GtkWidget *vbox = gtk_vbox_new(0,0);
			GtkWidget *e = gtk_entry_new();	
				gtk_entry_set_editable(GTK_ENTRY(e), false);
				gtk_widget_set_size_request(e, 75, -1);
				g_object_set_data(G_OBJECT(e), "fv_start_date", (gpointer)(is_old?date:0));
			gtk_box_pack_start(GTK_BOX(vbox), e, 1,1,0);
			
			c = gtk_calendar_new ();	
				if(date){
					int year, month, day;
					computeYMD(date, &day, &month, &year);
					gtk_calendar_select_month (GTK_CALENDAR(c), month - 1, year);
					gtk_calendar_select_day (GTK_CALENDAR(c), day);
				}
				g_signal_connect (G_OBJECT (c), "focus-out-event", G_CALLBACK (gtk_widget_hide), 0);	
				g_signal_connect (G_OBJECT (c), "day-selected-double-click", G_CALLBACK (saveDate2), e);		
				g_signal_connect (G_OBJECT (c), "day-selected", G_CALLBACK (updateCalendarEntry), e);		
				//~ g_signal_connect (G_OBJECT (c), "day-selected", G_CALLBACK (showNotImplementedDialog), this);		
			gtk_box_pack_start(GTK_BOX(vbox), c, 1,1,0);
			
			updateCalendarEntry(GTK_CALENDAR(c), GTK_ENTRY(e));
			
			g_signal_connect (G_OBJECT (e), "button-press-event", G_CALLBACK (showCalendarAndFocusOnIt), c);	
			g_signal_connect (G_OBJECT (e), "focus-in-event", G_CALLBACK (showCalendarAndFocusOnIt), c);	
			packer.packStart(vbox);		
		}
		{
			GtkWidget *e = gtk_entry_new();		
				char buf[100];
				snprintf(buf, sizeof(buf),"%g", value);
				gtk_entry_set_text(GTK_ENTRY(e), buf);
				value*=100000;
				int value_int = (int)value;
				g_object_set_data(G_OBJECT(e), "fv_value", (gpointer)(is_old?value_int:0));
				gtk_widget_set_size_request(e, 50, -1);
			//Ограничение на ввод: только числа и точки
			packer.packStart(e);
		}
	}
	gtk_widget_hide(c);	
	return c;
}

void AgrWindow::addSoz(int member_id, int type_id){
	HBoxToVBox packer(xml, "vboxSoz");
	{
		GtkWidget *e = gtk_combo_box_entry_new();
			gtk_combo_box_set_model(GTK_COMBO_BOX(e), GTK_TREE_MODEL(storeMember));
			gtk_combo_box_entry_set_text_column (GTK_COMBO_BOX_ENTRY(e), 1);			
			if(member_id)
				set_cb_data("fv_member_id", member_id, GTK_TREE_MODEL(storeMember), GTK_COMBO_BOX(e));
		packer.packStart(e);
	}
	{
		GtkWidget *e = gtk_combo_box_new();
			GtkCellRenderer *renderer = gtk_cell_renderer_text_new ();
			gtk_cell_layout_clear (GTK_CELL_LAYOUT (e));
			gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (e), renderer, TRUE);
			gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (e), renderer,
							"text", 1,
							NULL);			
			gtk_combo_box_set_model(GTK_COMBO_BOX(e), GTK_TREE_MODEL(storeSozTypes));					
			if(type_id)
				set_cb_data("fv_type_id", type_id, GTK_TREE_MODEL(storeSozTypes), GTK_COMBO_BOX(e));
		packer.packStart(e);
	}
}

void AgrWindow::bAddSozClicked(GtkButton *button, AgrWindow *aw){ aw->addSoz(0, 0); }
void AgrWindow::bAddMainRateClicked(GtkButton *button, AgrWindow *aw){ aw->addRate(0, 0, 1, false); }
void AgrWindow::bAddPrRateClicked(GtkButton *button, AgrWindow *aw){ aw->addRate(0, 0, 2, false); }

int AgrWindow::scheduleCanCreate(/*int *reason_id, */const char* eSum, financial *sum){
	if(agr_id <= 0){
		showErrDialog("Договор не сохранен");
		return -1;
	}		
	if(str2financial(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget(xml, eSum))), sum)<0){
		showErrDialog("Сумма должна быть числом > 0");
		return -3;
	}	
	return 0;
}

void AgrWindow::bCreatePaymentVydClicked(GtkWidget *button, AgrWindow *aw){
	bSaveClicked(button, aw);
	financial sum;
	if(aw->scheduleCanCreate("eSum",&sum)<0){
		return;
	}
	bool one_payment = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(glade_xml_get_widget(aw->xml, "cb1Payment")));
	financial annPaymentSum;
	if( !one_payment ){
		if(str2financial(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget(aw->xml, "eAnnPaymentSum"))), &annPaymentSum)<0){
			aw->showErrDialog("Аннуитетный платеж должен быть числом > 0");
			return;
		}		
	}
	PaymentProcessing pp(aw->agr_id, aw->db);
	string err = pp.createPaymentVyd(sum, annPaymentSum, 
			str2JD(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget(aw->xml, "eStartDate")))),
			str2JD(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget(aw->xml, "eEndDate")))),
			(bool)gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(glade_xml_get_widget(aw->xml, "cbFirstMonthPay"))), 
			one_payment, 
			gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget(aw->xml, "eComment")))
		);
	if(	!err.empty() ){
		aw->showErrDialog(("Ошибка создания платежа и графика:\n"+err).c_str());
		return;		
	}
	aw->schedule_refresh();
}

void AgrWindow::bCreatePaymentClicked(GtkButton *button, AgrWindow *aw){
	financial sum;
	if(aw->scheduleCanCreate("ePaySum", &sum)<0){
		return;
	}	
	int date = str2JD(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget(aw->xml, "ePayDate"))));	
	
	if(aw->db->isPaymentsAfterDate(aw->agr_id, date)!=0){
		aw->showErrDialog("Есть платежи после введенной даты");
		return;
	}
	PaymentProcessing pp(aw->agr_id, aw->db);
	std::string err = pp.createPayment(sum, date, gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget(aw->xml, "ePayComment"))));
	if(!err.empty()){
		aw->showErrDialog(err.c_str());
		return;
	}
	aw->schedule_refresh();
	if(pp.getAllDolg(date)==0){
			aw->db->close_agr(aw->agr_id, date);
	}
	aw->mainWindow->refresh();
}

void AgrWindow::bLoadFromCSVClicked(GtkButton *button, AgrWindow *aw){
	bool fPayment = (button == GTK_BUTTON(glade_xml_get_widget(aw->xml, "bLoadPaymentFromCSV")));
	financial sum;
	if(aw->scheduleCanCreate(fPayment?"ePaySum":"eSum", &sum)<0){
		return;
	}
	static gchar* dir=0;
	GtkWidget *dialog = gtk_file_chooser_dialog_new ("Открыть файл",
		GTK_WINDOW(aw->wMain),
		GTK_FILE_CHOOSER_ACTION_OPEN,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
		NULL);
	if(dir){
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), dir);
	}

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		if(dir){
			g_free(dir);
			dir=0;
		}
		dir=gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER (dialog));
		
		PaymentProcessing pp(aw->agr_id, aw->db);
		string err = 
			fPayment?
			pp.createPayment(sum, 
				str2JD(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget(aw->xml, "ePayDate")))),
				gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget(aw->xml, "ePayComment"))),
				filename):
			pp.createPaymentVyd(sum, 0, 
				str2JD(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget(aw->xml, "eStartDate")))),
				0,
				false, 
				false, 
				gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget(aw->xml, "eComment"))),
				filename
			);
			
		if(	!err.empty() ){			
			aw->showErrDialog(("Ошибка создания платежа и графика:\n"+err).c_str());
			return;		
		}
		aw->schedule_refresh();
		g_free (filename);
	}

	gtk_widget_destroy (dialog);
}

void AgrWindow::bSaveClicked(GtkWidget *button, AgrWindow *aw){
	//~ printf(__func__);
	
	GtkTreeIter iter;
//Сохраняем займодавца	
	int depositor_id = get_cb_data(GTK_COMBO_BOX(aw->cbeDepositor));
	if(!depositor_id){
		aw->showErrDialog("Займодавец не выбран");
		return;
	}
	if(aw->agr_id <= 0){
		aw->agr_id = aw->db->insert_fv_agr(depositor_id);
		
		if(aw->agr_id<=0){
			aw->showErrDialog("Договор не создан");
			return;
		}
	}else{
		int e = aw->db->update_fv_agr(aw->agr_id, depositor_id);
		
		if(e<0){
			aw->showErrDialog("Ошибка изменения договора");
			return;
		}
	}
//Сохраняем созаемщиков
{	
	GArray *ga4del = g_array_new (FALSE, FALSE, sizeof (int));

	GArray *ga4insert_id = g_array_new (FALSE, FALSE, sizeof (int));
	GArray *ga4insert_value = g_array_new (FALSE, FALSE, sizeof (int));

	GArray *ga4update_id = g_array_new (FALSE, FALSE, sizeof (int));
	GArray *ga4update_value = g_array_new (FALSE, FALSE, sizeof (int));

	GList*  vbox1ChildList = gtk_container_get_children(GTK_CONTAINER(glade_xml_get_widget (aw->xml, "vboxSoz")));
	for(GList* vbox1ChildListItem= g_list_first(vbox1ChildList); vbox1ChildListItem; vbox1ChildListItem =  g_list_next(vbox1ChildListItem)){
		if (GTK_IS_HBOX(vbox1ChildListItem->data)){			
			GList*  hboxChildList = gtk_container_get_children(GTK_CONTAINER(vbox1ChildListItem->data));
//			if(gtk_widget_get_visible(GTK_WIDGET(vbox1ChildListItem->data))){
			if(GTK_WIDGET_VISIBLE(vbox1ChildListItem->data)){
				if (GTK_IS_COMBO_BOX_ENTRY(hboxChildList->data)){					
					int member_id_new = get_cb_data(GTK_COMBO_BOX(hboxChildList->data));
					if(member_id_new == 0) //Если member_id не выбран
						continue;
					int member_id_old = (int)g_object_get_data(G_OBJECT(hboxChildList->data), "fv_member_id");
					//(member_id_new == member_id_old)
					//Если member_id совпадает, то update, иначе delete(если member_id!=0) и insert
					hboxChildList = g_list_next(hboxChildList);
					if (GTK_IS_COMBO_BOX(hboxChildList->data)){
							int type_id_new = get_cb_data(GTK_COMBO_BOX(hboxChildList->data));
							if(type_id_new == 0) //Если member_id не выбран
								continue;
							int type_id_old = (int)g_object_get_data(G_OBJECT(hboxChildList->data), "fv_type_id");
							
							if(member_id_new == member_id_old){
								if(type_id_new != type_id_old){
									g_array_append_val (ga4update_id, member_id_new);
									g_array_append_val (ga4update_value, type_id_new);									
								}//else - осталось без изменений
							}else{
								if(member_id_old){
									g_array_append_val (ga4del, member_id_old);
								}
								g_array_append_val (ga4insert_id, member_id_new);
								g_array_append_val (ga4insert_value, type_id_new);
							}
					}
				}
			}else{
				if (GTK_IS_COMBO_BOX_ENTRY(hboxChildList->data)){
					int member_id4del = (int)g_object_get_data(G_OBJECT(hboxChildList->data), "fv_member_id");
					if(member_id4del)
						g_array_append_val (ga4del, member_id4del);
				}
			}	
		}
	}

//Должен остаться хотя бы 1 пользователь. 
//Займодавец не может быть пользователем или поручителем


	for (int i = 0; i < ga4del->len; i++){
		//~ deleteSozaem(1234, g_array_index (ga4del, int, i));
		aw->db->delete_fv_sozaem(aw->agr_id, g_array_index (ga4del, int, i));
	}
	
	for (int i = 0; i < ga4update_id->len; i++){
		aw->db->update_fv_sozaem(aw->agr_id, g_array_index (ga4update_id, int, i), g_array_index (ga4update_id, int, i));		
	}
	for (int i = 0; i < ga4insert_id->len; i++){
		aw->db->insert_fv_sozaem(aw->agr_id, g_array_index (ga4insert_id, int, i), g_array_index (ga4insert_value, int, i));
	}
	g_array_free (ga4update_id, TRUE);
	g_array_free (ga4update_value, TRUE);
	g_array_free (ga4insert_id, TRUE);
	g_array_free (ga4insert_value, TRUE);
	g_array_free (ga4del, TRUE);	
}	

//Сохраняем график изменения ставок

for(int i_type=1;i_type<=2;i_type++)
{	
	GArray *ga4del = g_array_new (FALSE, FALSE, sizeof (int));

	GArray *ga4insert_id = g_array_new (FALSE, FALSE, sizeof (int));
	GArray *ga4insert_value = g_array_new (FALSE, FALSE, sizeof (double));

	GArray *ga4update_id = g_array_new (FALSE, FALSE, sizeof (int));
	GArray *ga4update_value = g_array_new (FALSE, FALSE, sizeof (double));

	GList*  vbox1ChildList = gtk_container_get_children(GTK_CONTAINER(glade_xml_get_widget (aw->xml, (i_type==1)?"vboxMainRate":"vboxPrRate")));
	for(GList* vbox1ChildListItem= g_list_first(vbox1ChildList); vbox1ChildListItem; vbox1ChildListItem =  g_list_next(vbox1ChildListItem)){
		if (GTK_IS_HBOX(vbox1ChildListItem->data)){			
			GList*  hboxChildList = gtk_container_get_children(GTK_CONTAINER(vbox1ChildListItem->data));
//			if(gtk_widget_get_visible(GTK_WIDGET(vbox1ChildListItem->data))){
			if(GTK_WIDGET_VISIBLE(vbox1ChildListItem->data)){
				if (GTK_IS_VBOX(hboxChildList->data)){
					GList*  vboxCalChildList = gtk_container_get_children(GTK_CONTAINER(hboxChildList->data));
					if (GTK_IS_ENTRY(vboxCalChildList->data)){					
						const char* start_date_new_str = gtk_entry_get_text(GTK_ENTRY(vboxCalChildList->data));
						int start_date_new_int = str2JD(start_date_new_str);
						//~ printf("%s %i\n", start_date_new_str, start_date_new_int);
						//~ if(value_new == 0) 
							//~ continue;
						int start_date_old_int = (int)g_object_get_data(G_OBJECT(vboxCalChildList->data), "fv_start_date");

						hboxChildList = g_list_next(hboxChildList);
						if (GTK_IS_ENTRY(hboxChildList->data)){
								double value_new = atof(gtk_entry_get_text(GTK_ENTRY(hboxChildList->data)));
								if(value_new == 0) 
									continue;
								double value_old = (int)g_object_get_data(G_OBJECT(hboxChildList->data), "fv_value");
								value_old/=100000;
								//~ printf("%f\n", value_new);
								if(start_date_new_int == start_date_old_int){
									if(value_new != value_old){
										g_array_append_val (ga4update_id, start_date_new_int);
										g_array_append_val (ga4update_value, value_new);									
									}//else - осталось без изменений
								}else{
									if(start_date_old_int){
										g_array_append_val (ga4del, start_date_old_int);
									}
									g_array_append_val (ga4insert_id, start_date_new_int);
									g_array_append_val (ga4insert_value, value_new);
								}
						}
					}
				}
			}else{
				if (GTK_IS_VBOX(hboxChildList->data)){
					GList*  vboxCalChildList = gtk_container_get_children(GTK_CONTAINER(hboxChildList->data));
					if (GTK_IS_ENTRY(vboxCalChildList->data)){
						int start_date_old_int = (int)g_object_get_data(G_OBJECT(vboxCalChildList->data), "fv_start_date");
						if(start_date_old_int)
							g_array_append_val (ga4del, start_date_old_int);
					}
				}
			}	
		}
	}

//Должен остаться хотя бы 1 пользователь. 
//Займодавец не может быть пользователем или поручителем


	for (int i = 0; i < ga4del->len; i++){
		aw->db->delete_fv_rate(aw->agr_id, g_array_index (ga4del, int, i), i_type);
	}
	
	for (int i = 0; i < ga4update_id->len; i++){
		aw->db->update_fv_rate(aw->agr_id, g_array_index (ga4update_id, int, i), i_type, g_array_index (ga4update_value, double, i));
	}
	for (int i = 0; i < ga4insert_id->len; i++){
		aw->db->insert_fv_rate(aw->agr_id, g_array_index (ga4insert_id, int, i), i_type, g_array_index (ga4insert_value, double, i));
	}
	g_array_free (ga4update_id, TRUE);
	g_array_free (ga4update_value, TRUE);
	g_array_free (ga4insert_id, TRUE);
	g_array_free (ga4insert_value, TRUE);
	g_array_free (ga4del, TRUE);	
	
}


	MainWindow*mw = aw->mainWindow;
	mw->refresh();
	if (button==aw->bSave){	
		DB* _db = aw->db;	
		int agr_id = aw->agr_id;
		bool closed = aw->closed;
		gtk_widget_destroy(GTK_WIDGET(aw->wMain));
		AgrWindow* agrW = new AgrWindow(mw, _db, agr_id, depositor_id, closed);	
	}
}


int AgrWindow::callback_fv_sozaem(void* param, int id, PARAMS_fv_sozaem){
	AgrWindow* aw = (AgrWindow*)param;
	aw->addSoz(id, type_id);
}

int AgrWindow::callback_fv_schedules(void* param, int id, int start_date, int reason_id, financial oper_sum, const char* comment){
	GtkListStore *store = (GtkListStore *)param;
	GtkTreeIter iter;
    gtk_list_store_append (store, &iter);
    char start_date_str[2+1+2+1+4 + 1];
    JD2str(start_date, start_date_str);
    gtk_list_store_set (store, &iter,
		schedule_col_number, id, 
		schedule_col_start_date, start_date_str,
		schedule_col_reason, reasonTypes[reason_id-1],
		schedule_col_oper_sum, financial2str(oper_sum).c_str(),
		schedule_col_comment, comment,
		  -1);
}

//~ void AgrWindow::cbeDepositorChanged(GtkWidget *widget, AgrWindow* aw){
	//~ if (!aw->agr_id){
		//~ int depositor_id = get_cb_data(GTK_COMBO_BOX(widget));	//!!Убрать
		//~ aw->agr_id = aw->db->insert_fv_agr(depositor_id);
		//~ char buf[256];
		//~ snprintf(buf,sizeof(buf),"Договор номер %i", aw->agr_id);
		//~ gtk_window_set_title(aw->wMain, buf);	
		//~ aw->schedule_refresh();	
	//~ }
//~ }

AgrWindow::AgrWindow(MainWindow* _mainWindow, DB* _db, int _agr_id, int depositor_id, bool _closed, bool standard_template):
	db(_db),mainWindow(_mainWindow),agr_id(_agr_id),schedule_state_model(0), closed(_closed)
	, Base("agrWindow")
	{		
	
	if (!xml) {
		g_warning("Failed to create the interface: agrWindow");
		return;
	}
	

	glade_xml_signal_autoconnect(xml);
	//gtk_window_maximize(wMain);
	
	
	storeMember = gtk_list_store_new (2,G_TYPE_UINT,G_TYPE_STRING);	
	db->select_orv_member("order by fio", callback_orv_member, storeMember);	

	cbeDepositor = glade_xml_get_widget(xml, "cbeDepositor");
	gtk_combo_box_set_model(GTK_COMBO_BOX(cbeDepositor), GTK_TREE_MODEL(storeMember));
	gtk_combo_box_entry_set_text_column (GTK_COMBO_BOX_ENTRY(cbeDepositor), 1);
	g_object_unref (G_OBJECT(storeMember));	

	set_cb_data("fv_depositor_id", depositor_id, GTK_TREE_MODEL(storeMember), GTK_COMBO_BOX(cbeDepositor));

	//~ g_signal_connect (G_OBJECT (cbeDepositor), "changed", G_CALLBACK (cbeDepositorChanged), this);	

	storeSozTypes = gtk_list_store_new (2,G_TYPE_UINT,G_TYPE_STRING);	
	GtkTreeIter iter;
    gtk_list_store_append (storeSozTypes, &iter);
    gtk_list_store_set (storeSozTypes, &iter,
		  0, 1,
		  1, "Пользователь",
		  -1);	
	gtk_list_store_append (storeSozTypes, &iter);
    gtk_list_store_set (storeSozTypes, &iter,
		  0, 2,
		  1, "Поручитель",
		  -1);	
	
	
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_clear (GTK_CELL_LAYOUT (cbeDepositor));
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (cbeDepositor), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (cbeDepositor), renderer,
					"text", 1,
					NULL);

	gtk_widget_show_all(GTK_WIDGET(wMain));
	expanderVydacha = glade_xml_get_widget(xml, "expanderVydacha");
	expanderPogashenie = glade_xml_get_widget(xml, "expanderPogashenie");	
	gtk_widget_hide(expanderVydacha);
	gtk_widget_hide(expanderPogashenie);
	
	for(int i=0;i<3;i++){				
		GtkWidget *calStartDate = glade_xml_get_widget(xml, (i==1)?"calStartDate":((i==2)?"calEndDate":"calPayDate"));
		GtkWidget *eStartDate=glade_xml_get_widget(xml, (i==1)?"eStartDate":((i==2)?"eEndDate":"ePayDate"));

		time_t t = time(0);
		struct tm* TM = localtime(&t);

		gtk_calendar_select_month (GTK_CALENDAR(calStartDate), TM->tm_mon, TM->tm_year+1900);
		gtk_calendar_select_day (GTK_CALENDAR(calStartDate), TM->tm_mday);


		gtk_widget_hide(calStartDate);
		gtk_entry_set_editable(GTK_ENTRY(eStartDate), false);
		g_signal_connect (G_OBJECT (calStartDate), "focus-out-event", G_CALLBACK (gtk_widget_hide), 0);	
		g_signal_connect (G_OBJECT (calStartDate), "day-selected-double-click", G_CALLBACK (saveDate2), eStartDate);		
		g_signal_connect (G_OBJECT (calStartDate), "day-selected", G_CALLBACK (updateCalendarEntry), eStartDate);
		if(i==0)//calPayDate
			g_signal_connect (G_OBJECT (calStartDate), "day-selected", G_CALLBACK (updateCalendarAllDolg), this);
			
		updateCalendarEntry(GTK_CALENDAR(calStartDate), GTK_ENTRY(eStartDate));
				
		g_signal_connect (G_OBJECT (eStartDate), "button-press-event", G_CALLBACK (showCalendarAndFocusOnIt), calStartDate);	
		g_signal_connect (G_OBJECT (eStartDate), "focus-in-event", G_CALLBACK (showCalendarAndFocusOnIt), calStartDate);			
	}	
	
	
	
	
	
	
	tvSchedule = glade_xml_get_widget(xml, "tvSchedule");
	for(int i=0;i<schedule_num_cols;i++){		
		GtkCellRenderer *renderer = gtk_cell_renderer_text_new ();
		if(i==schedule_col_oper_sum){
			GValue val ={ 0, { { 0 } } };// G_VALUE_INIT;
			g_value_init (&val, G_TYPE_FLOAT);
			g_value_set_float(&val, 1);
			g_object_set_property (G_OBJECT (renderer), "xalign", &val);
			g_value_unset (&val);		
		}
		GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes (schedule_headers[i],
								 renderer,
								 "text", i,
								 NULL);
		gtk_tree_view_column_set_resizable(column, true);
		gtk_tree_view_column_set_min_width(column, 10);								 
		gtk_tree_view_column_set_sort_column_id (column, i);
		gtk_tree_view_append_column (GTK_TREE_VIEW(tvSchedule), column);
	}	
	tvScheduleState = glade_xml_get_widget(xml, "tvScheduleState");
	for(int i=0;i<schedule_state_num_cols;i++){		
		GtkCellRenderer *renderer;
		//~ if(i == schedule_state_col_fact){
			//~ renderer = gtk_cell_renderer_toggle_new();  
			//~ g_signal_connect (renderer, "toggled", G_CALLBACK (factToggled), this);			
		//~ }else{
			renderer = gtk_cell_renderer_text_new ();		
			//~ g_object_set(G_OBJECT(renderer), "xalign",1, NULL);
			
			GValue val ={ 0, { { 0 } } };// G_VALUE_INIT;
			g_value_init (&val, G_TYPE_FLOAT);
			g_value_set_float(&val, (i == schedule_state_col_comment)?0:1);
			g_object_set_property (G_OBJECT (renderer), "xalign", &val);
			g_value_unset (&val);
		//~ }
		GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes (schedule_state_headers[i],
								 renderer,
								 "text", i,
								 NULL);
		gtk_tree_view_column_set_resizable(column, true);
		gtk_tree_view_column_set_min_width(column, 10);								 
		gtk_tree_view_column_set_sort_column_id (column, i);
		gtk_tree_view_append_column (GTK_TREE_VIEW(tvScheduleState), column);
	}	
	
{
	GtkWidget *hboxCalAllDolg = glade_xml_get_widget(xml, "hboxCalAllDolg");
	eAllDolg = glade_xml_get_widget(xml, "eAllDolg");
	elAllDolg = glade_xml_get_widget(xml, "elAllDolg");
	
	calAllDolg = GTK_CALENDAR(gtk_calendar_new ());	
		g_signal_connect (G_OBJECT (calAllDolg), "focus-out-event", G_CALLBACK (gtk_widget_hide), 0);	
		g_signal_connect (G_OBJECT (calAllDolg), "day-selected-double-click", G_CALLBACK (saveDate2AllDolg), this);		
		g_signal_connect (G_OBJECT (calAllDolg), "day-selected", G_CALLBACK (updateCalendarEntryAllDolg), this);		
	gtk_box_pack_start(GTK_BOX(hboxCalAllDolg), GTK_WIDGET(calAllDolg), 0,0,0);
	
	//~ updateCalendarEntryAllDolg(calAllDolg, this);
	
	g_signal_connect (G_OBJECT (eAllDolg), "button-press-event", G_CALLBACK (showCalendarAndFocusOnIt), calAllDolg);	
	g_signal_connect (G_OBJECT (eAllDolg), "focus-in-event", G_CALLBACK (showCalendarAndFocusOnIt), calAllDolg);		
}	
	for(int i=0;i<sizeof(calStartDate)/sizeof(*calStartDate);i++)
		calStartDate[i]=0;
	calStartDate[1]=GTK_CALENDAR(glade_xml_get_widget(xml, "calStartDate"));
	if(agr_id){
		char buf[256];
		snprintf(buf,sizeof(buf),"Договор номер %i", agr_id);
		gtk_window_set_title(wMain, buf);
		db->select_fv_sozaem(agr_id, callback_fv_sozaem, this);
		db->select_fv_rate(agr_id, callback_fv_rate, this);
		schedule_refresh();
		calStartDate[2]=0;
	}else{
		gtk_window_set_title(wMain, "Новый договор");
		addSoz(0, 1);
		GObject* mainCalendar;
		int jdnow = JDnow();		
		if(standard_template){
			mainCalendar = G_OBJECT (addRate(jdnow, 20, 1, false));
			calStartDate[2]=GTK_CALENDAR(addRate(add_months(jdnow, 1), 13, 1, false));
			calStartDate[0]=GTK_CALENDAR(addRate(jdnow, 40, 2, false));
		}else{
			mainCalendar = G_OBJECT (addRate(jdnow, 0, 1, false));
			calStartDate[0]=GTK_CALENDAR(addRate(jdnow, 0, 2, false));
		}
		
		g_signal_connect (mainCalendar, "day-selected", G_CALLBACK (onUpdateMainCalendar), this);
	}
	
	
	if(closed){
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget(xml, "bCloseAgr")),"Перевести в действующие");
	}
	

	GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(tvSchedule));
	gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);
	g_signal_connect (selection,"changed",		G_CALLBACK (tvSchedule_selection_changed),		this);		
	GtkTreeSelection *selectionSS = gtk_tree_view_get_selection (GTK_TREE_VIEW(tvScheduleState));
	gtk_tree_selection_set_mode (selectionSS, GTK_SELECTION_SINGLE);
	g_signal_connect (selectionSS,"changed",		G_CALLBACK (tvSchedule_state_selection_changed),		this);		
	g_signal_connect (glade_xml_get_widget(xml, "bAddSoz"), "clicked",  G_CALLBACK (bAddSozClicked), this);
	g_signal_connect (glade_xml_get_widget(xml, "bAddMainRate"), "clicked",  G_CALLBACK (bAddMainRateClicked), this);				
	g_signal_connect (glade_xml_get_widget(xml, "bAddPrRate"), "clicked",  G_CALLBACK (bAddPrRateClicked), this);				
	g_signal_connect (bSave = glade_xml_get_widget(xml, "bSave"), "clicked",  G_CALLBACK (bSaveClicked), this);				
	g_signal_connect (glade_xml_get_widget(xml, "bCreatePaymentVyd"), "clicked",  G_CALLBACK (bCreatePaymentVydClicked), this);				
	g_signal_connect (glade_xml_get_widget(xml, "bCreatePayment"), "clicked",  G_CALLBACK (bCreatePaymentClicked), this);				
	g_signal_connect (glade_xml_get_widget(xml, "bLoadFromCSV"), "clicked",  G_CALLBACK (bLoadFromCSVClicked), this);				
	g_signal_connect (glade_xml_get_widget(xml, "bLoadPaymentFromCSV"), "clicked",  G_CALLBACK (bLoadFromCSVClicked), this);				
	g_signal_connect (glade_xml_get_widget(xml, "bDelPayment"), "clicked",  G_CALLBACK (bDelPaymentClicked), this);				
	g_signal_connect (glade_xml_get_widget(xml, "bCalcAnnPayment"), "clicked",  G_CALLBACK (bCalcAnnPaymentClicked), this);				
	g_signal_connect (glade_xml_get_widget(xml, "bCloseAgr"), "clicked",  G_CALLBACK (bCloseAgrClicked), this);				
	g_signal_connect (glade_xml_get_widget(xml, "bRaspText"), "clicked",  G_CALLBACK (bRaspTextClicked), this);
	g_signal_connect (glade_xml_get_widget(xml, "bAgrText"), "clicked",  G_CALLBACK (bAgrTextClicked), this);
	g_signal_connect (glade_xml_get_widget(xml, "cb1Payment"), "toggled",  G_CALLBACK (cb1PaymentToggled), this);
	g_signal_connect (glade_xml_get_widget(xml, "bSoglas"), "clicked",  G_CALLBACK (showNotImplementedDialog), this);	
	g_signal_connect (wMain, "destroy",  G_CALLBACK (onDestroy), this);				
}

void AgrWindow::updateCalendarAllDolg(GtkCalendar *calendar, AgrWindow* aw){
	guint year, month, day;
	gtk_calendar_get_date(calendar,&year,&month,&day);
	gtk_calendar_select_month (aw->calAllDolg, month, year);
	gtk_calendar_select_day (aw->calAllDolg, day);
}

void AgrWindow::onUpdateMainCalendar(GtkCalendar *calendar, AgrWindow* aw){
	guint year, month, day;
	gtk_calendar_get_date(calendar,&year,&month,&day);
	for(int i=0;i<2;i++)
		if(aw->calStartDate[i]){
			gtk_calendar_select_month (aw->calStartDate[i], month, year);
			gtk_calendar_select_day (aw->calStartDate[i], day);
		}

	if(aw->calStartDate[2]){
		int jd = computeJD(day, month+1, year);
		jd = add_months(jd, 1);
		computeYMD(jd, (int*)&day, (int*)&month, (int*)&year);
	
		gtk_calendar_select_month (aw->calStartDate[2], month-1, year);
		gtk_calendar_select_day (aw->calStartDate[2], day);	
	}
}

void AgrWindow::cb1PaymentToggled(GtkToggleButton *togglebutton, AgrWindow *aw){
	GtkWidget* cbFirstMonthPay = glade_xml_get_widget(aw->xml, "cbFirstMonthPay");
	GtkWidget* hboxPl = glade_xml_get_widget(aw->xml, "hboxPl");
	if(gtk_toggle_button_get_active(togglebutton)){
		gtk_widget_hide_all (cbFirstMonthPay);
		gtk_widget_hide_all (hboxPl);		
	}else{
		gtk_widget_show_all (cbFirstMonthPay);
		gtk_widget_show_all (hboxPl);
	}
}

void AgrWindow::bAgrTextClicked(GtkButton *button, AgrWindow *aw){
	agr2odt(aw->agr_id, aw->db);
	aw->showInfoDialog("Текст договора сформирован");
}

void AgrWindow::bCloseAgrClicked(GtkButton *button, AgrWindow *aw){
  if(aw->closed){
	  aw->db->reopen_agr(aw->agr_id);
	  aw->mainWindow->refresh();
  }else{
	  GtkWidget *dialog = gtk_dialog_new_with_buttons ("Введите дату закрытия договора",
						GTK_WINDOW (aw->wMain),
						GtkDialogFlags(GTK_DIALOG_MODAL| GTK_DIALOG_DESTROY_WITH_PARENT),
						GTK_STOCK_OK,
						GTK_RESPONSE_OK,
						GTK_STOCK_CANCEL,
						GTK_RESPONSE_CANCEL,
						0);

	  GtkWidget *hbox = gtk_hbox_new (FALSE, 8);
	  gtk_container_set_border_width (GTK_CONTAINER (hbox), 8);
	  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), hbox, FALSE, FALSE, 0);

	  GtkWidget *stock = gtk_image_new_from_stock (GTK_STOCK_DIALOG_QUESTION, GTK_ICON_SIZE_DIALOG);
	  gtk_box_pack_start (GTK_BOX (hbox), stock, FALSE, FALSE, 0);

	  GtkWidget *table = gtk_table_new (2, 2, FALSE);
	  gtk_table_set_row_spacings (GTK_TABLE (table), 4);
	  gtk_table_set_col_spacings (GTK_TABLE (table), 4);
	  gtk_box_pack_start (GTK_BOX (hbox), table, TRUE, TRUE, 0);
	  GtkWidget *label = gtk_label_new ("Дата закрытия");
	  gtk_table_attach_defaults (GTK_TABLE (table),
					 label,
					 0, 1, 0, 1);
	  GtkWidget *local_cal = gtk_calendar_new  ();
	  g_signal_connect (G_OBJECT (local_cal), "day-selected-double-click", G_CALLBACK (closeDateSelected), dialog);		
	  gtk_table_attach_defaults (GTK_TABLE (table), local_cal, 1, 2, 0, 1);
	  
	  gtk_widget_show_all (hbox);

	  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_OK) {
		guint D, M, Y;
		gtk_calendar_get_date(GTK_CALENDAR(local_cal), &Y, &M, &D);
		aw->db->close_agr(aw->agr_id, computeJD(D, M+1, Y));
		aw->mainWindow->refresh();
	  }

	  gtk_widget_destroy (dialog);	
  }	
}

void AgrWindow::bDelPaymentClicked(GtkButton *button, AgrWindow *aw){
	if(aw->showAcceptDialog("Вы уверены, что хотите удалить последний платеж?")== GTK_RESPONSE_YES)
		aw->db->delete_last_fv_payment(aw->agr_id);
	aw->schedule_refresh();
}

void AgrWindow::bRaspTextClicked(GtkButton *button, AgrWindow *aw){
	
	financial sum=aw->db->getAgrSum(aw->agr_id)/100;
	char ssum[500];
	try
    {
		snprintf(ssum, sizeof(ssum),"%i", (int)sum);
		std::string s = string_functions::number_in_words(ssum);
		snprintf(ssum, sizeof(ssum),"%i (%s)", (int)sum, s.c_str());
    }
    catch (std::exception& e)
    {
		printf("%s %s\n", e.what(), ssum);
		return;
    }
    
	char str_date[2+1+2+1+4 + 1];
	int date = aw->db->getSigned(aw->agr_id);
	JD2str(date, str_date);	
	
	vector<SMem> vSMem;
	aw->db->getMemberAgrData(aw->agr_id, &vSMem);
	if(vSMem.size()<1+1){
		aw->showErrDialog("Не заданы займодавец и хотя бы один созаемщик");
		return;
	}
			
	char buf[1000];
	snprintf(buf, sizeof(buf), 
		"Договор мной прочитан. Смысл и содержание понятны. Со штрафными санкциями соглас%s.\n"
		"Денежная сумма %s рублей получена.\n"
		"%s Подпись / %s",((vSMem[1].pol)?"ен":"на"), ssum, str_date,vSMem[1].fio.c_str());
	aw->showInfoDialog(buf);
}

void AgrWindow::bCalcAnnPaymentClicked(GtkButton *button, AgrWindow *aw){
	financial sum;
	if(aw->scheduleCanCreate("eSum", &sum) < 0){
		return;
	}
	
	PaymentProcessing s(aw->agr_id, aw->db);
	financial ann_plat = s.getAnnValue(sum
	, str2JD(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget(aw->xml, "eStartDate"))))
	, str2JD(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget(aw->xml, "eEndDate"))))
	, gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(glade_xml_get_widget(aw->xml, "cbFirstMonthPay")))
	);

    gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget(aw->xml, "eAnnPaymentSum")), financial2str(ann_plat).c_str());
}

void AgrWindow::tvSchedule_state_selection_changed (GtkTreeSelection *selection, AgrWindow *aw)
{
	GtkTreeModel *model;
	GtkTreeIter iter;	
	if(gtk_tree_selection_get_selected (selection, &model, &iter)){
		char *c_date;
		gtk_tree_model_get (model, &iter,
			schedule_state_col_date, &c_date,
		-1);
		if(*c_date == 0)
			aw->selected_schedule_state_date = 0;
		else
			aw->selected_schedule_state_date = str2JD(c_date);
		
		g_free(c_date);
	}	
}

int AgrWindow::callback_select_fv_schedule_and_payment_states (void* param, 
		int state_date, int state_kind, financial remainder_sched, financial interest_sched, financial payment_sched,
		financial payment_all, financial remainder_overdue, financial interest_overdue, financial payment_overdue, const char* comment, int payment_id){
	AgrWindow* aw = (AgrWindow *)param;
	GtkListStore *storeState = (GtkListStore *)(aw->schedule_state_model);
	aw->sum_proc+=interest_sched;
	if(payment_sched>0)
		aw->sum_plat+=payment_sched;
	aw->sum_plat_fact+=payment_all;
	aw->sum_proc_pr+=interest_overdue;
	aw->sum_plat_pr+=payment_overdue;
	GtkTreeIter iter;		  				  
	gtk_list_store_append (storeState, &iter);
    char state_date_str[2+1+2+1+4 + 1];
    JD2str(state_date, state_date_str);

	gtk_list_store_set (storeState, &iter,
		schedule_state_col_date, state_date_str,
		schedule_state_col_remainder_sched, (state_kind==FACT_LINE)?"":financial2str(remainder_sched).c_str(),
		schedule_state_col_interest_sched, (state_kind==FACT_LINE)?"":financial2str(interest_sched).c_str(),
		schedule_state_col_payment_sched, (state_kind==FACT_LINE)?"":financial2str(payment_sched).c_str(),
		schedule_state_col_payment_all, (state_kind==PLAN_LINE)?"":financial2str(payment_all).c_str(),
		schedule_state_col_remainder_overdue, (state_kind==PLAN_LINE)?"":financial2str(remainder_overdue).c_str(),
		schedule_state_col_interest_overdue, (state_kind==PLAN_LINE)?"":financial2str(interest_overdue).c_str(),
		schedule_state_col_payment_overdue, (state_kind==PLAN_LINE)?"":financial2str(payment_overdue).c_str(),
		schedule_state_col_comment, comment,
		schedule_state_col_payment_num, payment_id,
		  -1);	
}


void AgrWindow::tvSchedule_selection_changed (GtkTreeSelection *selection, AgrWindow *aw)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	int sel = gtk_tree_selection_get_selected (selection, &model, &iter);
	if(sel){
		gtk_tree_model_get (model, &iter,
			schedule_col_number, &(aw->selected_schedule_id),
			-1);
			
		GtkListStore *storeState = gtk_list_store_new (schedule_state_num_cols,
					  G_TYPE_STRING,
					  G_TYPE_STRING,
					  G_TYPE_STRING,
					  G_TYPE_STRING,
					  G_TYPE_STRING,
					  G_TYPE_STRING,
					  G_TYPE_STRING,
					  G_TYPE_STRING,
					  G_TYPE_STRING,
					  G_TYPE_UINT);
		aw->schedule_state_model=GTK_TREE_MODEL (storeState); 
		aw->sum_proc = aw->sum_plat = aw->sum_plat_fact = aw->sum_proc_pr = aw->sum_plat_pr = 0;		
		//~ aw->db->select_fv_schedule_state(aw->selected_schedule_id, callback_fv_schedule_state, aw);
		aw->db->select_fv_schedule_and_payment_states(aw->agr_id, aw->selected_schedule_id, callback_select_fv_schedule_and_payment_states, aw);

		GtkTreeIter iterA;		  				  
		gtk_list_store_append (storeState, &iterA);
		gtk_list_store_set (storeState, &iterA,
			schedule_state_col_date, "",
			schedule_state_col_remainder_sched, "Итого:",
			schedule_state_col_interest_sched, financial2str(aw->sum_proc).c_str(),
			schedule_state_col_payment_sched, financial2str(aw->sum_plat).c_str(),
			schedule_state_col_payment_all, financial2str(aw->sum_plat_fact).c_str(),
			schedule_state_col_payment_overdue, financial2str(aw->sum_plat_pr).c_str(), //str_overdue,
			schedule_state_col_interest_overdue, financial2str(aw->sum_proc_pr).c_str(), //str_overdue_interest,
			schedule_state_col_comment, "",
			schedule_state_col_payment_num,0,
			  -1);

		gtk_tree_view_set_model(GTK_TREE_VIEW(aw->tvScheduleState), aw->schedule_state_model);
		g_object_unref (G_OBJECT(storeState));		
	}else{
		aw->selected_schedule_id = 0;
		if(aw->schedule_state_model)
			gtk_list_store_clear (GTK_LIST_STORE(aw->schedule_state_model));
	}
}


void AgrWindow::onDestroy(GtkWidget *widget,AgrWindow *aw){
//	printf(__func__);	
	aw->mainWindow->refresh();
	delete aw;
}



int AgrWindow::callback_orv_member(void* _store, int id, PARAMS_orv_member){
	GtkListStore *store = (GtkListStore *)_store;
	GtkTreeIter iter;
    gtk_list_store_append (store, &iter);
    gtk_list_store_set (store, &iter,
		  0, id,
		  1, fio,
		  -1);
	return 0;
}


int AgrWindow::callback_fv_rate(void* param, PARAMS_ID_fv_rate, PARAMS_fv_rate){
	AgrWindow* aw = (AgrWindow*)param;
	GObject* mainCalendar = G_OBJECT (aw->addRate(id, value, type, true));
	if((type==1)&&(aw->calStartDate[2]==0)){
		aw->calStartDate[2]=GTK_CALENDAR(mainCalendar);		
		g_signal_connect (mainCalendar, "day-selected", G_CALLBACK (onUpdateMainCalendar), aw);
		
		guint year, month, day;
		gtk_calendar_get_date(aw->calStartDate[2],&year,&month,&day);
		if(aw->calStartDate[1]){
			gtk_calendar_select_month (aw->calStartDate[1], month, year);
			gtk_calendar_select_day (aw->calStartDate[1], day);
		}
	}
		
	if((type==2)&&(aw->calStartDate[0]==0))
		aw->calStartDate[0]=GTK_CALENDAR(mainCalendar);		
}

AgrWindow::~AgrWindow(){	
//	printf(__func__);	
	g_object_unref (G_OBJECT(storeSozTypes));		
}
