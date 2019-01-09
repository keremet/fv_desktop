#include <gtk/gtk.h>
#include <glade/glade.h>
#include "db.h"
#include "MainWindow.h"

int main(int argc, char **argv) {
	gtk_init(&argc, &argv);
	glade_init();
		
	DB db;
	MainWindow mw(&db);
	
    gtk_main();
    return 0;
}

/*
 * TODO
 * Выявить и убрать утечки памяти (delete на каждый new)
 * */
