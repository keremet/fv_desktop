#ifndef DB_H
#define DB_H
#include "sqlite-amalgamation-3071602/sqlite3.h"
#include "financial.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>

#define PLAN_LINE 1
#define FACT_LINE 2
#define ALL_LINE 3	//FACT_LINE|PLAN_LINE

#define PARAMS_orv_member const char* fio, bool pol_m, const char* pasp_ser, const char* pasp_num, int pasp_dvyd, const char* pasp_kem_vyd, const char* addr_reg, const char* contact, const char* rel_code, financial fv_zayavka, const char* comment
#define PARAMS_FILTER_orv_member const char* filter

#define PARAMS_fv_agr int depositor_id
#define PARAMS_fv_agr_select const char* depositor, int depositor_id, int signed_date, const char* users, const char* poruch, financial sum, int end_date, int closed
#define PARAMS_FILTER_fv_agr const char* filter

#define PARAMS_fv_schedule int payment_id, int reason_id
#define PARAMS_FILTER_fv_schedule int payment_id

#define PARAMS_FILTER_fv_schedule_state int schedule_id
#define PARAMS_fv_schedule_state  int schedule_id, int state_date, financial remainder, financial interest, financial payment, const char* comment, int payment_id

#define PARAMS_FILTER_fv_payment int agr_id
#define PARAMS_fv_payment  int agr_id, int state_date, financial all_payment, financial overdue_remainder, financial overdue_interest, financial overdue_payment, const char* comment


#define FUNC_BLOCK_U_I_D(NAME) int insert_##NAME(PARAMS_##NAME); \
	int update_##NAME(int id, PARAMS_##NAME); \
	int delete_##NAME(int id);

#define FUNC_BLOCK(NAME) typedef int(*callback_##NAME)(void* param, int id, PARAMS_##NAME); \
	int select_##NAME(PARAMS_FILTER_##NAME, callback_##NAME cb, void* param); \
	FUNC_BLOCK_U_I_D(NAME)
	
#define PARAMS_fv_sozaem int type_id
#define PARAMS_fv_rate double value
#define PARAMS_fv_pr_rate double value
#define PARAMS_ID_fv_rate int id, int type
#define PARAMS_ID_fv_sozaem int id


#define FUNC_BLOCK_AGR_PROP(NAME)  typedef int(*callback_##NAME)(void* param, PARAMS_ID_##NAME, PARAMS_##NAME); \
	int select_##NAME(int agr_id, callback_##NAME cb, void* param); \
	int insert_##NAME(int agr_id, PARAMS_ID_##NAME, PARAMS_##NAME); \
	int update_##NAME(int agr_id,  PARAMS_ID_##NAME, PARAMS_##NAME); \
	int delete_##NAME(int agr_id, PARAMS_ID_##NAME);

typedef struct {
	int date;
	int schedule_id;
	financial value;
} SchedPayment;

typedef struct{
	bool pol;
	std::string fio;
	std::string info;
} SMem;

class DB{
	sqlite3 *db;
	sqlite3_stmt* stmt_insert_fv_schedule_state;
	
	static int step_and_finalize(sqlite3_stmt *st);
public:
	DB();
	~DB();
	FUNC_BLOCK(orv_member) 
	FUNC_BLOCK_U_I_D(fv_agr) 
	
	FUNC_BLOCK_U_I_D(fv_schedule) 
	FUNC_BLOCK(fv_schedule_state) 
	FUNC_BLOCK(fv_payment) 
	
	FUNC_BLOCK_AGR_PROP(fv_sozaem)
	FUNC_BLOCK_AGR_PROP(fv_rate)

	typedef int(*callback_fv_agr)(void* param, int id, PARAMS_fv_agr_select); 
	int select_fv_agr(PARAMS_FILTER_fv_agr, callback_fv_agr cb, void* param); 
	
	typedef int(*callback_select_fv_schedule_and_payment_states)(void* param, 
		int state_date, int state_kind, financial remainder_sched, financial interest_sched, financial payment_sched,
		financial payment_all, financial remainder_overdue, financial interest_overdue, financial payment_overdue, const char* comment, int payment_id); 
	int select_fv_schedule_and_payment_states(int agr_id, int schedule_id, callback_select_fv_schedule_and_payment_states cb, void* param);
	
	typedef int(*callback_fv_schedules)(void* param, int id, int start_date, int reason_id, financial oper_sum, const char* comment); 
	int select_fv_schedules(int agr_id, callback_fv_schedules cb, void* param); 
	
	int close_agr(int agr_id, int date);
	int reopen_agr(int agr_id);
	int get_prev_schedule_id(int agr_id, int schedule_id);
	int state_after_last_movements(int agr_id, int date, int *rem_date, financial* rem, int *overdue_date, financial* overdue);
	int isPaymentsAfterDate(int agr_id, int date);
	int getMissedPayments(int agr_id, int date, std::vector<SchedPayment> *vp);
	int bind_payment2schedule_state(int schedule_id, int state_date, int payment_id);
	int delete_last_fv_payment(int agr_id);
	int getSchedPaymentOnDate(int agr_id, int date, SchedPayment* sp);
	int getLastScheduleID(int agr_id);
	financial getAgrSum(int agr_id);
	int getSigned(int agr_id);
	int getPlanEndDate(int agr_id);
	int getMemberAgrData(int agr_id, std::vector<SMem> *v);
};

#endif
