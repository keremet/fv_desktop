#include "db.h"
#include "base.h"
#include <glade/glade.h>
class MembersWindow: public Base{
	DB* db;
	static const char* headers[];	
	enum
	{
	  col_num,
	  col_fio,
	  col_pol_m,
	  col_pasp_ser,
	  col_pasp_num,
	  col_pasp_dvyd, 
	  col_pasp_kem_vyd, 
	  col_addr_reg, 
	  col_contact, 
	  col_fv_zayavka, 
	  col_comment,
	  num_cols
	};	
	int selected_id;
	static int callback_orv_member(void* mw, int id, PARAMS_orv_member);
	static void bAddClicked(GtkButton *button, MembersWindow *mw);
	static void bChangeClicked(GtkButton *button, MembersWindow *mw);
	static void bDelClicked(GtkButton *button, MembersWindow *mw);
	static void selection_changed (GtkTreeSelection *selection, MembersWindow *mw);
	void refresh();
public:
	MembersWindow(DB* _db);
	~MembersWindow();
};
