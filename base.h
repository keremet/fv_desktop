/*
 * base.h
 *
 *  Created on: 19.10.2010
 *      Author: keremet
 */

#ifndef BASE_H_
#define BASE_H_

#include <gtk/gtk.h>
#include <glade/glade.h>

class Base{
	static gboolean wEscPressed (GtkWidget   *widget, GdkEventKey *event); //Закрытие дочернего окна при нажатии ESC	
protected:	
	static void showNotImplementedDialog(GtkWidget   *widget, Base* b); //Отображение информации, что функциональность не реализована
public:
	GtkWindow *wMain;
	GladeXML *xml;
	Base(const char* window_name);
	gint showAcceptDialog(const gchar* msg); //Отображение диалога Да/Нет
	gint showErrDialog(const gchar* msg); //Отображение информации об ошибке
	gint showInfoDialog(const gchar* msg); //Отображение информации об ошибке
};

#endif /* BASE_H_ */
