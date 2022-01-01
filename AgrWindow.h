#include "db.h"
#include "MainWindow.h"
#include "base.h"
#include "financial.h"
#include <glade/glade.h>

class AgrWindow: public Base{
	MainWindow* mainWindow;
	DB* db;	
	GtkListStore *storeMember;
	GtkListStore *storeSozTypes;
	GtkTreeModel *schedule_model;
	GtkTreeModel *schedule_state_model;
	GtkWidget* cbeDepositor;
	GtkWidget* tvSchedule;
	GtkWidget* tvScheduleState;
	GtkWidget* eAllDolg;
	GtkWidget* elAllDolg;
	GtkWidget *expanderVydacha;
	GtkWidget *expanderPogashenie;
	GtkWidget *bSave;
	GtkCalendar *calAllDolg;
	GtkCalendar* calStartDate[3];
	enum{
		schedule_col_number,
		schedule_col_start_date,
		schedule_col_reason,
		schedule_col_oper_sum,
		schedule_col_comment,
		schedule_num_cols
	};
	enum{
		schedule_state_col_date,
		schedule_state_col_remainder_sched,
		schedule_state_col_interest_sched,
		schedule_state_col_payment_sched,
		schedule_state_col_payment_all,
		schedule_state_col_remainder_overdue,
		schedule_state_col_interest_overdue,
		schedule_state_col_payment_overdue,
		schedule_state_col_comment,
		schedule_state_col_payment_num,
		schedule_state_num_cols
	};
	static const char* schedule_headers[];
	static const char* schedule_state_headers[];
	int agr_id;
	bool closed;
	int selected_schedule_id;
	int selected_schedule_state_date;
	void addSoz(int member_id, int type_id);
	GtkWidget* addRate(int date, double value, int type_id, bool is_old);
	void schedule_refresh();
	int scheduleCanCreate(const char* eSum, financial *sum);
	
	financial sum_proc;
	financial sum_plat;
	financial sum_plat_fact;
	financial sum_proc_pr;
	financial sum_plat_pr;
	
	static int callback_orv_member(void* mw, int id, PARAMS_orv_member);
	static void callback_fv_sozaem(void* param, int id, PARAMS_fv_sozaem);
	static void callback_fv_schedules(void* param, int id, int start_date, int reason_id, financial oper_sum, const char* comment);
	static void callback_fv_rate(void* param, PARAMS_ID_fv_rate, PARAMS_fv_rate);
	static int callback_fv_schedule_state(void* param, int id, PARAMS_fv_schedule_state);
	static void callback_select_fv_schedule_and_payment_states (void* param, 
		int state_date, int state_kind, financial remainder_sched, financial interest_sched, financial payment_sched, 
		financial payment_all, financial remainder_overdue, financial interest_overdue, financial payment_overdue, const char* comment, int payment_id);
	static void tvSchedule_selection_changed(GtkTreeSelection *selection, AgrWindow *aw);
	static void tvSchedule_state_selection_changed(GtkTreeSelection *selection, AgrWindow *aw);
	static void bAddSozClicked(GtkButton *button, AgrWindow *aw);
	static void bAddMainRateClicked(GtkButton *button, AgrWindow *aw);
	static void bAddPrRateClicked(GtkButton *button, AgrWindow *aw);
	static void bSaveClicked(GtkWidget *button, AgrWindow *aw);
	static void bCreatePaymentVydClicked(GtkWidget *button, AgrWindow *aw);
	static void bCreatePaymentClicked(GtkButton *button, AgrWindow *aw);
	static void bLoadFromCSVClicked(GtkButton *button, AgrWindow *aw);
	static void bCloseAgrClicked(GtkButton *button, AgrWindow *aw);
	static void bDelPaymentClicked(GtkButton *button, AgrWindow *aw);
	static void bCalcAnnPaymentClicked(GtkButton *button, AgrWindow *aw);
	static void bRaspTextClicked(GtkButton *button, AgrWindow *aw);
	static void bAgrTextClicked(GtkButton *button, AgrWindow *aw);
	static void cb1PaymentToggled(GtkToggleButton *togglebutton, AgrWindow *aw);
	static void onDestroy(GtkWidget *widget,AgrWindow *aw);
	//~ static void cbeDepositorChanged(GtkWidget *widget,AgrWindow *aw);
	static void updateCalendarEntryAllDolg(GtkCalendar *calendar, AgrWindow* aw);
	static void onUpdateMainCalendar(GtkCalendar *calendar, AgrWindow* aw);
	static void updateCalendarAllDolg(GtkCalendar *calendar, AgrWindow* aw);
	static void saveDate2AllDolg (GtkCalendar *calendar, AgrWindow* aw);
public:
	AgrWindow(MainWindow* _mainWindow, DB* _db, int agr_id, int depositor_id, bool _closed, bool standard_template=false);
	~AgrWindow();
};
