/*
 * base.cpp
 *
 *  Created on: 19.10.2010
 *      Author: keremet
 */

#include "base.h"
#include <gdk/gdkkeysyms.h>

Base::Base(const char* window_name):
	xml(glade_xml_new("interface.glade", window_name, NULL))
{
	wMain = GTK_WINDOW(glade_xml_get_widget(xml, window_name));
	g_signal_connect (wMain, "key-press-event",  G_CALLBACK (wEscPressed), 0);
}

gboolean Base::wEscPressed (GtkWidget   *widget, GdkEventKey *event){
	if(event->keyval==GDK_Escape)
		gtk_widget_destroy(widget);
	return false;
}

gint Base::showAcceptDialog(const gchar* msg){
	GtkWidget*  dlg =  gtk_message_dialog_new              (wMain,
															 GTK_DIALOG_MODAL,
															 GTK_MESSAGE_QUESTION,
															 GTK_BUTTONS_YES_NO,
															 msg);

	gint result = gtk_dialog_run (GTK_DIALOG (dlg));
	gtk_widget_destroy (dlg);
	return result;
}

gint Base::showErrDialog(const gchar* msg){
	GtkWidget*  dlg =  gtk_message_dialog_new              (wMain,
															 GTK_DIALOG_MODAL,
															 GTK_MESSAGE_ERROR,
															 GTK_BUTTONS_OK,
															 msg);

	gint result = gtk_dialog_run (GTK_DIALOG (dlg));
	gtk_widget_destroy (dlg);
	return result;
}


gint Base::showInfoDialog(const gchar* msg){
	GtkWidget*  dlg =  gtk_message_dialog_new              (wMain,
															 GTK_DIALOG_MODAL,
															 GTK_MESSAGE_INFO,
															 GTK_BUTTONS_OK,
															 msg);

	gint result = gtk_dialog_run (GTK_DIALOG (dlg));
	gtk_widget_destroy (dlg);
	return result;
}

void Base::showNotImplementedDialog(GtkWidget   *widget, Base* b){
	b->showInfoDialog("Функциональность будет доступна в следующих версиях");
}
