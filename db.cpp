#include "db.h"
#include "julianday.h"
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
//~ #include <map>

using namespace std;


#define sqlite3_column_financial sqlite3_column_int64
#define sqlite3_bind_financial sqlite3_bind_int64

int DB::step_and_finalize(sqlite3_stmt *st){
	int r=0;
	if(sqlite3_step(st)!=SQLITE_DONE)
		r=-1;
	sqlite3_finalize(st);
	return r;	
}

DB::DB():stmt_insert_fv_schedule_state(0){
	if( sqlite3_open_v2("db.sqlite", &db, SQLITE_OPEN_READWRITE,0) ){
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return;
	}
	
	sqlite3_exec(db,"PRAGMA foreign_keys = ON;", 0,0,0);
}

DB::~DB(){
	if(stmt_insert_fv_schedule_state)
		sqlite3_finalize(stmt_insert_fv_schedule_state);
	sqlite3_close(db);
}

int DB::select_orv_member(const char* filter, callback_orv_member cb, void* param){
	sqlite3_stmt *st;
	if(sqlite3_prepare_v2(db,
			(string("select id, fio, pol_m, pasp_ser, pasp_num, pasp_dvyd, pasp_kem_vyd, addr_reg, "
				"contact, rel_code, fv_zayavka, comment"
				" from orv_member ")
				+(filter?filter:"")).c_str(),
			-1, &st, 0)!=SQLITE_OK){
		fprintf(stderr, "%s error at sqlite3_prepare_v2 %s\n", __PRETTY_FUNCTION__, sqlite3_errmsg(db));
		return -1;
	}
	while(sqlite3_step(st)==SQLITE_ROW){
		cb(param, sqlite3_column_int(st,0), 
		(const char*)sqlite3_column_text(st,1),  
		 sqlite3_column_int(st,2), 
		(const char*)sqlite3_column_text(st,3), 
		(const char*)sqlite3_column_text(st,4), 	
		sqlite3_column_int(st,5), 	
		(const char*)sqlite3_column_text(st,6), 
		(const char*)sqlite3_column_text(st,7), 
		(const char*)sqlite3_column_text(st,8), 
		(const char*)sqlite3_column_text(st,9), 
		sqlite3_column_financial(st,10),
		(const char*)sqlite3_column_text(st,11));
	}
	sqlite3_finalize(st);
	return 0;
}

int DB::select_fv_schedules(int agr_id, callback_fv_schedules cb, void* param){
	sqlite3_stmt *st;
	if(sqlite3_prepare_v2(db,
			"select id, state_date, reason_id, abs(all_payment), comment from fv_payment"
			" left join fv_schedule on id=payment_id"
			" where fv_payment.agr_id=? and reason_id is not null"
			" order by state_date",
			-1, &st, 0)!=SQLITE_OK){
		fprintf(stderr, "%s error at sqlite3_prepare_v2 %s\n", __PRETTY_FUNCTION__, sqlite3_errmsg(db));
		return -1;
	}
	sqlite3_bind_int(st, 1, agr_id);
	while(sqlite3_step(st)==SQLITE_ROW){
		cb(param, sqlite3_column_int(st,0),
			sqlite3_column_int(st,1), 
			sqlite3_column_int(st,2), 
			sqlite3_column_financial(st,3), 
			(const char*)sqlite3_column_text(st,4)
		);
	}
	sqlite3_finalize(st);
	return 0;
}

int DB::getMemberAgrData(int agr_id, vector<SMem> *v){
	sqlite3_stmt *st;
	if(sqlite3_prepare_v2(db,
			"select depositor_id, type_id, pol_m, fio, addr_reg, pasp_ser, pasp_num, pasp_dvyd, pasp_kem_vyd from ("
			"	select depositor_id, 0 type_id from fv_agr "
			"	where id=?"
			"	union"
			"	select member_id, type_id from fv_sozaem"
			"	where agr_id=?"
			") a"
			" left join orv_member on id=a.depositor_id"
			" order by type_id",
			-1, &st, 0)!=SQLITE_OK){
		fprintf(stderr, "%s error at sqlite3_prepare_v2 %s\n", __PRETTY_FUNCTION__, sqlite3_errmsg(db));
		return -1;
	}
	sqlite3_bind_int(st, 1, agr_id);
	sqlite3_bind_int(st, 2, agr_id);
	while(sqlite3_step(st)==SQLITE_ROW){
		SMem s;
		s.pol = sqlite3_column_int(st,2);
		s.info = s.fio = (const char*)sqlite3_column_text(st,3);
		s.info += ", зарегистрированн";
		s.info += (s.pol)?"ый":"ая";
		s.info += " по адресу ";
		s.info += (const char*)sqlite3_column_text(st,4);
		s.info += ", паспорт серия ";
		s.info += (const char*)sqlite3_column_text(st,5);
		s.info += " № ";
		s.info += (const char*)sqlite3_column_text(st,6);
		s.info += " выдан ";
		
		int date = sqlite3_column_int(st,7);
		char str_date[100];
		JD2full_str(date, str_date, sizeof(str_date));
		s.info += str_date;
		s.info += " года ";
		s.info += (const char*)sqlite3_column_text(st,8);
		
		v->push_back(s);
	}
	sqlite3_finalize(st);	
	return 0;
	
//~ 
//~ ;
	//~ 
	//~ 
	//~ 
//~ //0 - займодавец	
	//~ s.pol=1;
	//~ s.info="1Соколов Андрей Юрьевич, зарегистрированный по адресу г. Киров, ул. Чапаева, д. 5, кв. 71, паспорт серия 3304 № 532894 выдан 31 мая 2005 года УВД Ленинского района г. Кирова";
	//~ v->push_back(s);
//~ //1 - первый созаемщик	
	//~ s.pol=1;
	//~ s.info="2Черепов Антон Андреевич, зарегистрированный по адресу Кировская обл, г. Киров, ул. Ульяновская, д.6, кв. 57, паспорт серия 3310 № 055765 выдан 17 февраля 2011 года Отделом УФМС России по Кировской области в Октябрьском районе города Кирова";
	//~ v->push_back(s);
//~ /*	s.pol=1;
	//~ s.info="Кряжев Игорь Александрович, зарегистрированный по адресу Кировская обл., г. Киров, ул. Космонавта Волкова, д.3, кв. 168, паспорт серия 3309 №983419 выдан 29 января 2010 года отделом УФМС России по Кировской области в Ленинском районе города Кирова";
	//~ v->push_back(s);	*/
}

int DB::select_fv_agr(const char* filter, callback_fv_agr cb, void* param){
	std::ostringstream ss;
	ss <<   "select fv_agr.id, orv_member.fio, fv_agr.depositor_id"
			", (select min(state_date) from fv_payment where agr_id=fv_agr.id)" //Дата подписания
			", (select group_concat(orv_member.fio||' ('||orv_member.id||')', ', ') from fv_sozaem"	//Пользователи
			"    left join orv_member on orv_member.id=fv_sozaem.member_id"
			"   where agr_id=fv_agr.id and type_id=1 "
			"  )"
			", (select group_concat(orv_member.fio||' ('||orv_member.id||')', ', ') from fv_sozaem"	//Поручители
			"    left join orv_member on orv_member.id=fv_sozaem.member_id"
			"   where agr_id=fv_agr.id and type_id=2 "
			"  )"
			", (select -sum(all_payment) from fv_payment "
			"     where all_payment<0 and agr_id=fv_agr.id  )" //Выдано
			", (select max(state_date) from fv_schedule_state"	//Плановая дата окончания
			"   where remainder=0 and schedule_id=a.last_schedule_id"
			"  )"
			", closed"
			" from fv_agr "
			" left join orv_member on orv_member.id=fv_agr.depositor_id"
			" left join ("
			"   select agr_id, max(payment_id) last_schedule_id from fv_schedule "
			"		left join fv_payment on id=payment_id"
			"   group by agr_id"
			" ) a on a.agr_id=fv_agr.id" << filter;	
	sqlite3_stmt *st;
	if(sqlite3_prepare_v2(db, ss.str().c_str(), -1, &st, 0)!=SQLITE_OK){
		fprintf(stderr, "%s error at sqlite3_prepare_v2 %s\n", __PRETTY_FUNCTION__, sqlite3_errmsg(db));
		return -1;
	}
	int r=-1;
	while(sqlite3_step(st)==SQLITE_ROW){
		financial sum = sqlite3_column_financial(st,6);
		//~ if(remainder == 0)
			//~ remainder = sum;
		cb(param, sqlite3_column_int(st,0), (const char*)sqlite3_column_text(st,1), sqlite3_column_int(st,2), sqlite3_column_int(st,3), (const char*)sqlite3_column_text(st,4), (const char*)sqlite3_column_text(st,5), sum, sqlite3_column_int(st,7), sqlite3_column_int(st,8));
	}
	sqlite3_finalize(st);
	return r;
}



int DB::update_fv_agr(int id, PARAMS_fv_agr){
	sqlite3_stmt *st;
	if(sqlite3_prepare_v2(db,
			"update fv_agr set depositor_id = ?"
			" where id=?",
			-1, &st, 0)!=SQLITE_OK){
		fprintf(stderr, "%s error at sqlite3_prepare_v2 %s\n", __PRETTY_FUNCTION__, sqlite3_errmsg(db));
		return -1;
	}
	sqlite3_bind_int(st, 1, depositor_id);
	sqlite3_bind_int(st, 2, id);
	return step_and_finalize(st);
}


int DB::insert_orv_member(PARAMS_orv_member){
	sqlite3_stmt *st;
	if(sqlite3_prepare_v2(db,
			"insert into orv_member(fio, pol_m, pasp_ser, pasp_num, pasp_dvyd, pasp_kem_vyd, addr_reg, contact, rel_code, fv_zayavka, comment)"
			"values(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
			-1, &st, 0)!=SQLITE_OK){
		fprintf(stderr, "%s error at sqlite3_prepare_v2 %s\n", __PRETTY_FUNCTION__, sqlite3_errmsg(db));
		return -1;
	}
	sqlite3_bind_text(st, 1, fio, -1, SQLITE_STATIC);
	sqlite3_bind_int(st, 2, pol_m);
	sqlite3_bind_text(st, 3, pasp_ser, -1, SQLITE_STATIC);
	sqlite3_bind_text(st, 4, pasp_num, -1, SQLITE_STATIC);
	sqlite3_bind_int(st, 5, pasp_dvyd);
	sqlite3_bind_text(st, 6, pasp_kem_vyd, -1, SQLITE_STATIC);
	sqlite3_bind_text(st, 7, addr_reg, -1, SQLITE_STATIC);
	sqlite3_bind_text(st, 8, contact, -1, SQLITE_STATIC);
	sqlite3_bind_text(st, 9, rel_code, -1, SQLITE_STATIC);
	sqlite3_bind_financial(st, 10, fv_zayavka);
	sqlite3_bind_text(st, 11, comment, -1, SQLITE_STATIC);
	return step_and_finalize(st);
}

#define DELETE_BY_ID(NAME) int DB::delete_##NAME(int id){ \
	sqlite3_stmt *st; \
	if(sqlite3_prepare_v2(db, \
			"delete from "#NAME" where id = ?", \
			-1, &st, 0)!=SQLITE_OK){ \
		fprintf(stderr, "%s error at sqlite3_prepare_v2 %s\n", __PRETTY_FUNCTION__, sqlite3_errmsg(db)); \
		return -1; \
	} \
	sqlite3_bind_int(st, 1, id); \
	return step_and_finalize(st);	\
}
DELETE_BY_ID(orv_member)
DELETE_BY_ID(fv_agr)
//~ DELETE_BY_ID(fv_payment)
//~ DELETE_BY_ID(fv_schedule)

int DB::delete_last_fv_payment(int agr_id){ 
	sqlite3_stmt *st; 
	if(sqlite3_prepare_v2(db,
			"select max(id) from fv_payment"
			" where agr_id=?",
			-1, &st, 0)!=SQLITE_OK){
		fprintf(stderr, "%s error at sqlite3_prepare_v2 %s\n", __PRETTY_FUNCTION__, sqlite3_errmsg(db));
		return -1;
	}
	sqlite3_bind_int(st, 1, agr_id);
	int payment_id=0;
	if(sqlite3_step(st)==SQLITE_ROW){	
		payment_id = sqlite3_column_int(st,0);
	}
	sqlite3_finalize(st);
		
	
	if(sqlite3_prepare_v2(db,			
			"delete from fv_payment where id = ?", 
			-1, &st, 0)!=SQLITE_OK){ 
		fprintf(stderr, "%s error at sqlite3_prepare_v2 %s\n", __PRETTY_FUNCTION__, sqlite3_errmsg(db)); 
		return -1; 
	} 
	sqlite3_bind_int(st, 1, payment_id); 
	return step_and_finalize(st);
}


int DB::update_orv_member(int id, PARAMS_orv_member){
	sqlite3_stmt *st;
	if(sqlite3_prepare_v2(db,
			"update orv_member set fio=?"
			", pol_m=?"
			", pasp_ser=?"
			", pasp_num=?"
			", pasp_dvyd=?"
			", pasp_kem_vyd=?"
			", addr_reg=?"
			", contact=?"
			", rel_code=?"
			", fv_zayavka=?"
			", comment=?"
			" where id = ?",
			-1, &st, 0)!=SQLITE_OK){
		fprintf(stderr, "%s error at sqlite3_prepare_v2 %s\n", __PRETTY_FUNCTION__, sqlite3_errmsg(db));
		return -1;
	}
	sqlite3_bind_text(st, 1, fio, -1, SQLITE_STATIC);
	sqlite3_bind_int(st, 2, pol_m);
	sqlite3_bind_text(st, 3, pasp_ser, -1, SQLITE_STATIC);
	sqlite3_bind_text(st, 4, pasp_num, -1, SQLITE_STATIC);
	sqlite3_bind_int(st, 5, pasp_dvyd);
	sqlite3_bind_text(st, 6, pasp_kem_vyd, -1, SQLITE_STATIC);
	sqlite3_bind_text(st, 7, addr_reg, -1, SQLITE_STATIC);
	sqlite3_bind_text(st, 8, contact, -1, SQLITE_STATIC);
	sqlite3_bind_text(st, 9, rel_code, -1, SQLITE_STATIC);
	sqlite3_bind_financial(st, 10, fv_zayavka);
	sqlite3_bind_text(st, 11, comment, -1, SQLITE_STATIC);
	sqlite3_bind_int(st, 12, id);
	return step_and_finalize(st);
}



int DB::insert_fv_sozaem(int agr_id, int id, PARAMS_fv_sozaem){
	sqlite3_stmt *st;
	if(sqlite3_prepare_v2(db,
			"insert into fv_sozaem(agr_id, member_id, type_id)values(?,?,?)",
			-1, &st, 0)!=SQLITE_OK){
		fprintf(stderr, "%s error at sqlite3_prepare_v2 %s\n", __PRETTY_FUNCTION__, sqlite3_errmsg(db));
		return -1;
	}
	sqlite3_bind_int(st, 1, agr_id);
	sqlite3_bind_int(st, 2, id);
	sqlite3_bind_int(st, 3, type_id);
	return step_and_finalize(st);
}


int DB::update_fv_sozaem(int agr_id, int id, PARAMS_fv_sozaem){
	sqlite3_stmt *st;
	if(sqlite3_prepare_v2(db,
			"update fv_sozaem"
			" set type_id=?"
			" where agr_id=? and member_id=?",
			-1, &st, 0)!=SQLITE_OK){
		fprintf(stderr, "%s error at sqlite3_prepare_v2 %s\n", __PRETTY_FUNCTION__, sqlite3_errmsg(db));
		return -1;
	}
	sqlite3_bind_int(st, 1, type_id);
	sqlite3_bind_int(st, 2, agr_id);
	sqlite3_bind_int(st, 3, id);
	return step_and_finalize(st);
}


int DB::delete_fv_sozaem(int agr_id, int id){
	sqlite3_stmt *st;
	if(sqlite3_prepare_v2(db,
			"delete from fv_sozaem where agr_id=? and member_id=?",
			-1, &st, 0)!=SQLITE_OK){
		fprintf(stderr, "%s error at sqlite3_prepare_v2 %s\n", __PRETTY_FUNCTION__, sqlite3_errmsg(db));
		return -1;
	}
	sqlite3_bind_int(st, 1, agr_id);
	sqlite3_bind_int(st, 2, id);
	return step_and_finalize(st);
}

int DB::delete_fv_rate(int agr_id, PARAMS_ID_fv_rate){
	sqlite3_stmt *st;
	if(sqlite3_prepare_v2(db,
			"delete from fv_rate where agr_id=? and start_date=?  and type_id=?",
			-1, &st, 0)!=SQLITE_OK){
		fprintf(stderr, "%s error at sqlite3_prepare_v2 %s\n", __PRETTY_FUNCTION__, sqlite3_errmsg(db));
		return -1;
	}
	sqlite3_bind_int(st, 1, agr_id);
	sqlite3_bind_int(st, 2, id);
	sqlite3_bind_int(st, 3, type);
	return step_and_finalize(st);
}

int DB::select_fv_sozaem(int agr_id, callback_fv_sozaem cb, void* param){
	sqlite3_stmt *st;
	if(sqlite3_prepare_v2(db,
			"select member_id, type_id from fv_sozaem"
			" where agr_id=?"
			" order by type_id",
			-1, &st, 0)!=SQLITE_OK){
		fprintf(stderr, "%s error at sqlite3_prepare_v2 %s\n", __PRETTY_FUNCTION__, sqlite3_errmsg(db));
		return -1;
	}
	sqlite3_bind_int(st, 1, agr_id);
	while(sqlite3_step(st)==SQLITE_ROW){
		cb(param, sqlite3_column_int(st,0), sqlite3_column_int(st,1));
	}
	sqlite3_finalize(st);
	return 0;	
}

int DB::insert_fv_rate(int agr_id, PARAMS_ID_fv_rate, PARAMS_fv_rate){
	sqlite3_stmt *st;
	if(sqlite3_prepare_v2(db,
			"insert into fv_rate(agr_id, start_date, type_id, value)values(?,?,?,?)",
			-1, &st, 0)!=SQLITE_OK){
		fprintf(stderr, "%s error at sqlite3_prepare_v2 %s\n", __PRETTY_FUNCTION__, sqlite3_errmsg(db));
		return -1;
	}
	sqlite3_bind_int(st, 1, agr_id);
	sqlite3_bind_int(st, 2, id);
	sqlite3_bind_int(st, 3, type);
	sqlite3_bind_double(st, 4, value);
	return step_and_finalize(st);
}

int DB::update_fv_rate(int agr_id, PARAMS_ID_fv_rate, PARAMS_fv_rate){
	sqlite3_stmt *st;
	if(sqlite3_prepare_v2(db,
			"update fv_rate set value=?"
			" where agr_id=? and start_date=? and type_id=?",
			-1, &st, 0)!=SQLITE_OK){
		fprintf(stderr, "%s error at sqlite3_prepare_v2 %s\n", __PRETTY_FUNCTION__, sqlite3_errmsg(db));
		return -1;
	}
	sqlite3_bind_double(st, 1, value);
	sqlite3_bind_int(st, 2, agr_id);
	sqlite3_bind_int(st, 3, id);
	sqlite3_bind_int(st, 4, type);
	return step_and_finalize(st);	
}

int DB::select_fv_rate(int agr_id, callback_fv_rate cb, void* param){
	sqlite3_stmt *st;
	if(sqlite3_prepare_v2(db,
			"select start_date, type_id, value from fv_rate"
			" where agr_id=?"
			" order by start_date",
			-1, &st, 0)!=SQLITE_OK){
		fprintf(stderr, "%s error at sqlite3_prepare_v2 %s\n", __PRETTY_FUNCTION__, sqlite3_errmsg(db));
		return -1;
	}
	sqlite3_bind_int(st, 1, agr_id);
	while(sqlite3_step(st)==SQLITE_ROW){	
		cb(param, sqlite3_column_int(st,0), sqlite3_column_int(st,1), sqlite3_column_double(st,2));
	}
	sqlite3_finalize(st);
	return 0;	
}

int DB::insert_fv_schedule_state(PARAMS_fv_schedule_state){
	if(!stmt_insert_fv_schedule_state){
		if(sqlite3_prepare_v2(db,
				"insert into fv_schedule_state"
					"(schedule_id, state_date, "
					"remainder, interest, payment, "
					"comment, payment_id)"
				"values(?,?,?,?,?,?,?)",
				-1, &stmt_insert_fv_schedule_state, 0)!=SQLITE_OK){
			fprintf(stderr, "%s error at sqlite3_prepare_v2 %s\n", __PRETTY_FUNCTION__, sqlite3_errmsg(db));
			return -1;
		}
	}
	sqlite3_bind_int(stmt_insert_fv_schedule_state, 1, schedule_id);
	sqlite3_bind_int(stmt_insert_fv_schedule_state, 2, state_date);
	sqlite3_bind_financial(stmt_insert_fv_schedule_state, 3, remainder);
	sqlite3_bind_financial(stmt_insert_fv_schedule_state, 4, interest);
	sqlite3_bind_financial(stmt_insert_fv_schedule_state, 5, payment);
	sqlite3_bind_text(stmt_insert_fv_schedule_state, 6, comment, -1, SQLITE_STATIC);
	if(payment_id)
		sqlite3_bind_int(stmt_insert_fv_schedule_state, 7, payment_id);
	else
		sqlite3_bind_null(stmt_insert_fv_schedule_state, 7);
	int r=0;
	if(sqlite3_step(stmt_insert_fv_schedule_state)!=SQLITE_DONE){
		fprintf(stderr, "%s error at sqlite3_step %s\n", __PRETTY_FUNCTION__, sqlite3_errmsg(db));
		r=-1;
	}
	sqlite3_reset(stmt_insert_fv_schedule_state);
	return r;		
}

int DB::select_fv_schedule_state(PARAMS_FILTER_fv_schedule_state, callback_fv_schedule_state cb, void* param){
	sqlite3_stmt *st;
	if(sqlite3_prepare_v2(db,
			"select state_date, remainder,interest,payment"
			" from fv_schedule_state"
			" where schedule_id=? and (payment_id is null or payment_id=0)"
			" order by state_date",
			-1, &st, 0)!=SQLITE_OK){
		fprintf(stderr, "%s error at sqlite3_prepare_v2 %s\n", __PRETTY_FUNCTION__, sqlite3_errmsg(db));
		return -1;
	}
	sqlite3_bind_int(st, 1, schedule_id);
	while(sqlite3_step(st)==SQLITE_ROW){
		cb(param, 0, 
			schedule_id,
			sqlite3_column_int(st,0), 
			sqlite3_column_financial(st,1), 
			sqlite3_column_financial(st,2), 
			sqlite3_column_financial(st,3), 			
			(const char*)sqlite3_column_text(st,4),
			sqlite3_column_int(st,5)
		);
	}
	sqlite3_finalize(st);
	return 0;
	
}

#define returnCurrVal(TABLE) 	if(sqlite3_prepare_v2(db, \
			"select seq from sqlite_sequence where name='"#TABLE"'", \
			-1, &st, 0)!=SQLITE_OK){ \
		fprintf(stderr, "%s error at sqlite3_prepare_v2 %s\n", __PRETTY_FUNCTION__, sqlite3_errmsg(db)); \
		return -1; \
	} \
	r=-1; \
	if(sqlite3_step(st)==SQLITE_ROW){ \
		r = sqlite3_column_int(st,0); \
	} \
	sqlite3_finalize(st); \
	return r;	

//~ int getCurrVal_fv_payment(sqlite3 *db){
	//~ sqlite3_stmt *st;
	//~ int r;
	//~ returnCurrVal(fv_payment)
//~ }


int DB::insert_fv_agr(PARAMS_fv_agr){
	sqlite3_stmt *st;
	if(sqlite3_prepare_v2(db,
			"insert into fv_agr(depositor_id)values(?)",
			-1, &st, 0)!=SQLITE_OK){
		fprintf(stderr, "%s error at sqlite3_prepare_v2 %s\n", __PRETTY_FUNCTION__, sqlite3_errmsg(db));
		return -1;
	}
	sqlite3_bind_int(st, 1, depositor_id);
	int r;
	if((r = step_and_finalize(st))<0)
		return r;
		
	returnCurrVal(fv_agr)	
}


int DB::insert_fv_payment(PARAMS_fv_payment){
	sqlite3_stmt *st;
	if(sqlite3_prepare_v2(db,
			"insert into fv_payment(agr_id, state_date, all_payment, overdue_remainder,"
			" overdue_interest, overdue_payment, comment)"
			"values(?, ?, ?, ?, ?, ?, ?)",
			-1, &st, 0)!=SQLITE_OK){
		fprintf(stderr, "%s error at sqlite3_prepare_v2 %s\n", __PRETTY_FUNCTION__, sqlite3_errmsg(db));
		return -1;
	}
	sqlite3_bind_int(st, 1, agr_id);
	sqlite3_bind_int(st, 2, state_date);
	sqlite3_bind_financial(st, 3, all_payment);
	sqlite3_bind_financial(st, 4, overdue_remainder);
	sqlite3_bind_financial(st, 5, overdue_interest);
	sqlite3_bind_financial(st, 6, overdue_payment);
	sqlite3_bind_text(st, 7, comment, -1, SQLITE_STATIC);
	int r;
	if((r = step_and_finalize(st))<0)
		return r;
	
	returnCurrVal(fv_payment)
}

int DB::insert_fv_schedule(PARAMS_fv_schedule){
	sqlite3_stmt *st;
	//~ if(sqlite3_prepare_v2(db,
			//~ "insert into fv_payment(agr_id, state_date, all_payment, overdue_remainder, overdue_interest, overdue_payment, comment)"
			//~ "values(?,?,?,0,0,0,?)",
			//~ -1, &st, 0)!=SQLITE_OK){
		//~ fprintf(stderr, "%s error at sqlite3_prepare_v2 %s\n", __PRETTY_FUNCTION__, sqlite3_errmsg(db));
		//~ return -1;
	//~ }
	//~ sqlite3_bind_int(st, 1, agr_id);
	//~ sqlite3_bind_int(st, 2, start_date);
	//~ sqlite3_bind_double(st, 3, -oper_sum);
	//~ sqlite3_bind_text(st, 4, comment, -1, SQLITE_STATIC);
	//~ int r;
	//~ if((r = step_and_finalize(st))<0)
		//~ return r;
	//~ int payment_id = getCurrVal_fv_payment(db);




	if(sqlite3_prepare_v2(db,
			"insert into fv_schedule(payment_id, reason_id)"
			"values(?,?)",
			-1, &st, 0)!=SQLITE_OK){
		fprintf(stderr, "%s error at sqlite3_prepare_v2 %s\n", __PRETTY_FUNCTION__, sqlite3_errmsg(db));
		return -1;
	}
	sqlite3_bind_int(st, 1, payment_id);
	sqlite3_bind_int(st, 2, reason_id);
	int r;
	if((r = step_and_finalize(st))<0)
		return r;
	
	return payment_id;
}

//~ int DB::update_fv_schedule_state_fact(int schedule_id, int state_date, bool fact, const char* comment){
	//~ sqlite3_stmt *st;
	//~ if(sqlite3_prepare_v2(db,
			//~ "update fv_schedule_state"
			//~ " set fact=?, comment=?"
			//~ " where schedule_id=? and state_date=?",
			//~ -1, &st, 0)!=SQLITE_OK){
		//~ fprintf(stderr, "%s error at sqlite3_prepare_v2 %s\n", __PRETTY_FUNCTION__, sqlite3_errmsg(db));
		//~ return -1;
	//~ }
	//~ sqlite3_bind_int(st, 1, fact);
	//~ sqlite3_bind_text(st, 2, comment, -1, SQLITE_STATIC);
	//~ sqlite3_bind_int(st, 3, schedule_id);
	//~ sqlite3_bind_int(st, 4, state_date);
	//~ return step_and_finalize(st);	
//~ }

int DB::close_agr(int agr_id, int date){
	sqlite3_stmt *st;
	if(sqlite3_prepare_v2(db,
			"update fv_agr"
			" set closed=?"
			" where id=?",
			-1, &st, 0)!=SQLITE_OK){
		fprintf(stderr, "%s error at sqlite3_prepare_v2 %s\n", __PRETTY_FUNCTION__, sqlite3_errmsg(db));
		return -1;
	}
	sqlite3_bind_int(st, 1, date);
	sqlite3_bind_int(st, 2, agr_id);
	return step_and_finalize(st);	
}

int DB::reopen_agr(int agr_id){
	sqlite3_stmt *st;
	if(sqlite3_prepare_v2(db,
			"update fv_agr"
			" set closed=null"
			" where id=?",
			-1, &st, 0)!=SQLITE_OK){
		fprintf(stderr, "%s error at sqlite3_prepare_v2 %s\n", __PRETTY_FUNCTION__, sqlite3_errmsg(db));
		return -1;
	}
	sqlite3_bind_int(st, 1, agr_id);
	return step_and_finalize(st);	
}

int DB::get_prev_schedule_id(int agr_id, int schedule_id){
	sqlite3_stmt *st;
	if(sqlite3_prepare_v2(db,
			"select max(id) from fv_schedule s1 where start_date<=("
			"  select start_date from fv_schedule s3 where s3.id=?"
			") and agr_id=? and id<?"	,
			-1, &st, 0)!=SQLITE_OK){
		fprintf(stderr, "%s error at sqlite3_prepare_v2 %s\n", __PRETTY_FUNCTION__, sqlite3_errmsg(db));
		return -1;
	}
	sqlite3_bind_int(st, 1, schedule_id);
	sqlite3_bind_int(st, 2, agr_id);
	sqlite3_bind_int(st, 3, schedule_id);
	int r = -1;
	if(sqlite3_step(st)==SQLITE_ROW){
		r = sqlite3_column_int(st,0);
	}
	sqlite3_finalize(st);
	return r;
}

int DB::getLastScheduleID(int agr_id){
	sqlite3_stmt *st;
	if(sqlite3_prepare_v2(db,
			"select max(payment_id) from fv_schedule"
			" where payment_id in ("
			" select id from fv_payment"
			" where agr_id=?"
			")"
			,
			-1, &st, 0)!=SQLITE_OK){
		fprintf(stderr, "%s error at sqlite3_prepare_v2 %s\n", __PRETTY_FUNCTION__, sqlite3_errmsg(db));
		return -1;
	}	
	sqlite3_bind_int(st, 1, agr_id);
	int schedule_id=-1;
	if(sqlite3_step(st)==SQLITE_ROW){		
		schedule_id = sqlite3_column_int(st,0);
	}
	sqlite3_finalize(st);	
	return schedule_id;
}

financial DB::getAgrSum(int agr_id){
	sqlite3_stmt *st;
	if(sqlite3_prepare_v2(db,
			"select -sum(all_payment) from fv_payment"
			" where all_payment<0 and agr_id=?"	
			,
			-1, &st, 0)!=SQLITE_OK){
		fprintf(stderr, "%s error at sqlite3_prepare_v2 %s\n", __PRETTY_FUNCTION__, sqlite3_errmsg(db));
		return -1;
	}	
	sqlite3_bind_int(st, 1, agr_id);
	financial r=-1;
	if(sqlite3_step(st)==SQLITE_ROW){		
		r = sqlite3_column_financial(st,0);
	}
	sqlite3_finalize(st);	
	return r;	
}

int DB::getSigned(int agr_id){
	sqlite3_stmt *st;
	if(sqlite3_prepare_v2(db,
			"select min(state_date) from fv_payment"
			" where agr_id=?"	
			,
			-1, &st, 0)!=SQLITE_OK){
		fprintf(stderr, "%s error at sqlite3_prepare_v2 %s\n", __PRETTY_FUNCTION__, sqlite3_errmsg(db));
		return -1;
	}	
	sqlite3_bind_int(st, 1, agr_id);
	int r=-1;
	if(sqlite3_step(st)==SQLITE_ROW){		
		r = sqlite3_column_int(st,0);
	}
	sqlite3_finalize(st);	
	return r;		
}

int DB::getPlanEndDate(int agr_id){
	sqlite3_stmt *st;
	if(sqlite3_prepare_v2(db,
			"select max(state_date) from fv_schedule_state"	
			"  where remainder=0 and schedule_id=("
			"    select max(payment_id) from fv_schedule where payment_id in ("
			"      select id from fv_payment"
			"      where agr_id=?"
			"    )"
			"  )"
			,
			-1, &st, 0)!=SQLITE_OK){
		fprintf(stderr, "%s error at sqlite3_prepare_v2 %s\n", __PRETTY_FUNCTION__, sqlite3_errmsg(db));
		return -1;
	}	
	sqlite3_bind_int(st, 1, agr_id);
	int r=-1;
	if(sqlite3_step(st)==SQLITE_ROW){		
		r = sqlite3_column_int(st,0);
	}
	sqlite3_finalize(st);	
	return r;		
}


int DB::state_after_last_movements(int agr_id, int date, int *rem_date, financial* rem, int *overdue_date, financial* overdue){
	*rem_date=0;
	*rem=0;
	*overdue_date=0;
	*overdue=0;
	sqlite3_stmt *st;
	if(sqlite3_prepare_v2(db,
			"select state_date, remainder from fv_schedule_state"
			" where schedule_id=("
			"  select max(payment_id) from fv_schedule"
			"  where payment_id in ("
			"	 select id from fv_payment"
			"	 where agr_id=? and state_date<=?"
			"  )"
			" ) and payment_id>0 and state_date<=?"
			" order by state_date desc"
			,
			-1, &st, 0)!=SQLITE_OK){
		fprintf(stderr, "%s error at sqlite3_prepare_v2 %s\n", __PRETTY_FUNCTION__, sqlite3_errmsg(db));
		return -1;
	}
	sqlite3_bind_int(st, 1, agr_id);
	sqlite3_bind_int(st, 2, date);
	sqlite3_bind_int(st, 3, date);
	if(sqlite3_step(st)==SQLITE_ROW){		
		*rem_date = sqlite3_column_int(st,0);
		*rem = sqlite3_column_financial(st,1);
	}
	sqlite3_finalize(st);

	if(sqlite3_prepare_v2(db,
			"select state_date, overdue_remainder from fv_payment"
			" where agr_id=? and state_date<=?"
			" order by id desc"
			,
			-1, &st, 0)!=SQLITE_OK){
		fprintf(stderr, "%s error at sqlite3_prepare_v2 %s\n", __PRETTY_FUNCTION__, sqlite3_errmsg(db));
		return -1;
	}
	sqlite3_bind_int(st, 1, agr_id);
	sqlite3_bind_int(st, 2, date);
	if(sqlite3_step(st)==SQLITE_ROW){		
		*overdue_date = sqlite3_column_int(st,0);
		*overdue = sqlite3_column_financial(st,1);
	}
	sqlite3_finalize(st);	
	return 0;
}

//~ typedef struct {
	//~ int id;
	//~ double all_payment;
	//~ double overdue_remainder;
	//~ double overdue_interest;
	//~ double overdue_payment;
	//~ string comment;
//~ } Payment;

int DB::select_fv_schedule_and_payment_states(int agr_id, int schedule_id, callback_select_fv_schedule_and_payment_states cb, void* param){	
	sqlite3_stmt *st;
	//~ map<long long, Payment> mPayment;
	

	if(sqlite3_prepare_v2(db,		
			"select fv_payment.state_date, remainder, interest, payment,all_payment, overdue_remainder, overdue_interest, overdue_payment, fv_payment.comment, fv_payment.id"
			" from fv_payment"
			" left join fv_schedule_state on fv_payment.id=fv_schedule_state.payment_id"
			" where agr_id=? and ("
			" not exists (select 1 from fv_schedule where payment_id in(select id from fv_payment where agr_id=? and id>?))"	//Следующего графика нет
			" or fv_payment.id<(select min(payment_id) from fv_schedule where payment_id in(select id from fv_payment where agr_id=? and id>?))" //fv_payment.id<= id следующего графика
			")",
			-1, &st, 0)!=SQLITE_OK){
		fprintf(stderr, "%s error at sqlite3_prepare_v2 %s\n", __PRETTY_FUNCTION__, sqlite3_errmsg(db));
		return -1;
	}
	sqlite3_bind_int(st, 1, agr_id);
	sqlite3_bind_int(st, 2, agr_id);
	sqlite3_bind_int(st, 3, schedule_id);
	sqlite3_bind_int(st, 4, agr_id);
	sqlite3_bind_int(st, 5, schedule_id);
	while(sqlite3_step(st)==SQLITE_ROW){
		if(sqlite3_column_type(st,1)==SQLITE_NULL)	//remainder
			cb(param, 
				sqlite3_column_int(st,0), FACT_LINE, 0, 0, 0,
			sqlite3_column_financial(st,4),
			sqlite3_column_financial(st,5),
			sqlite3_column_financial(st,6),
			sqlite3_column_financial(st,7), (const char*)sqlite3_column_text(st,8), sqlite3_column_int(st,9));
		else 
			cb(param, 
				sqlite3_column_int(st,0), ALL_LINE, sqlite3_column_financial(st,1), 
				sqlite3_column_financial(st,2), 
				sqlite3_column_financial(st,3),
			sqlite3_column_financial(st,4),
			sqlite3_column_financial(st,5),
			sqlite3_column_financial(st,6),
			sqlite3_column_financial(st,7), (const char*)sqlite3_column_text(st,8), sqlite3_column_int(st,9));
	}
	sqlite3_finalize(st);
	
	
	if(sqlite3_prepare_v2(db,			
			"select state_date, remainder,interest,payment,comment"
			" from fv_schedule_state"
			" where schedule_id=? and (payment_id is null or payment_id=0)"
			" order by fv_schedule_state.state_date",
			-1, &st, 0)!=SQLITE_OK){
		fprintf(stderr, "%s error at sqlite3_prepare_v2 %s\n", __PRETTY_FUNCTION__, sqlite3_errmsg(db));
		return -1;
	}
	sqlite3_bind_int(st, 1, schedule_id);
	//~ sqlite3_bind_int(st, 2, schedule_id);
	//~ sqlite3_bind_int(st, 3, schedule_id);
	while(sqlite3_step(st)==SQLITE_ROW){
//отображение платежей между двумя основными (погашение просрочки)		
		//~ while i++
		
		//~ if(sqlite3_column_type(st,10)==SQLITE_NULL)
			cb(param, 
				sqlite3_column_int(st,0), PLAN_LINE, sqlite3_column_financial(st,1), 
				sqlite3_column_financial(st,2), 
				sqlite3_column_financial(st,3),				
			0, 0, 0, 0, (const char*)sqlite3_column_text(st,4),0);
		//~ else 
			//~ cb(param, 
				//~ sqlite3_column_int(st,0), ALL_LINE, sqlite3_column_double(st,1), 
				//~ sqlite3_column_double(st,2), 
				//~ sqlite3_column_double(st,3),
			//~ sqlite3_column_double(st,5),
			//~ sqlite3_column_double(st,6),
			//~ sqlite3_column_double(st,7),
			//~ sqlite3_column_double(st,8), (const char*)sqlite3_column_text(st,9), sqlite3_column_int(st,10));
	}
	sqlite3_finalize(st);
	return 0;
}


int DB::isPaymentsAfterDate(int agr_id, int date){
	sqlite3_stmt *st;
	if(sqlite3_prepare_v2(db, 
			"select count(1) from fv_payment where agr_id=? and state_date>?", 
			-1, &st, 0)!=SQLITE_OK){ 
		fprintf(stderr, "%s error at sqlite3_prepare_v2 %s\n", __PRETTY_FUNCTION__, sqlite3_errmsg(db)); 
		return -1; 
	}
	sqlite3_bind_int(st, 1, agr_id);
	sqlite3_bind_int(st, 2, date); 
	int r=-1; 
	if(sqlite3_step(st)==SQLITE_ROW){ 
		r = sqlite3_column_int(st,0); 
	} 
	sqlite3_finalize(st); 
	return r;	
};

int DB::getSchedPaymentOnDate(int agr_id, int date, SchedPayment* sp){
	sqlite3_stmt *st;
	sp->date=0;
	sp->schedule_id=0;
	sp->value=0;
	if(sqlite3_prepare_v2(db, 
			"select payment, schedule_id from fv_schedule_state"
			" where schedule_id=("
			"  select max(payment_id) from fv_schedule"
			"  where payment_id in ("
			"	 select id from fv_payment"
			"	 where agr_id=? " //Всегда интересует последний график, потому что иначе были платежи после и алгоритм до сюда не дойдет //"and state_date <=?"
			"  )"
			" )"
			" and state_date=? and payment_id is null", 
			-1, &st, 0)!=SQLITE_OK){ 
		fprintf(stderr, "%s error at sqlite3_prepare_v2 %s\n", __PRETTY_FUNCTION__, sqlite3_errmsg(db)); 
		return -1; 
	}
	sqlite3_bind_int(st, 1, agr_id);
	sqlite3_bind_int(st, 2, date); 
	int r=-1; 
	if(sqlite3_step(st)==SQLITE_ROW){ 
		sp->value = sqlite3_column_financial(st,0); 
		sp->schedule_id = sqlite3_column_int(st,1); 
		sp->date=date;
		r=0;
	} 
	sqlite3_finalize(st); 
	return r;	
};

int DB::getMissedPayments(int agr_id, int date, vector<SchedPayment> *vp){
	sqlite3_stmt *st;
	if(sqlite3_prepare_v2(db,	
		"select state_date,schedule_id, payment from fv_schedule_state"
		" where schedule_id=("
		"  select max(payment_id) from fv_schedule"
		"  where payment_id in ("
		"	 select id from fv_payment"
		"	 where agr_id=?"
		"  )"
		" ) and (payment_id is null or payment_id=0) and state_date <?"
		" order by state_date",
			-1, &st, 0)!=SQLITE_OK){
		fprintf(stderr, "%s error at sqlite3_prepare_v2 %s\n", __PRETTY_FUNCTION__, sqlite3_errmsg(db));
		return -1;
	}
	sqlite3_bind_int(st, 1, agr_id);
	sqlite3_bind_int(st, 2, date);
	while(sqlite3_step(st)==SQLITE_ROW){
		SchedPayment p;
		p.date=sqlite3_column_int(st,0);
		p.schedule_id=sqlite3_column_int(st,1);
		p.value=sqlite3_column_financial(st,2);
		vp->push_back(p);
	}
	sqlite3_finalize(st);
	return 0;
}

int DB::bind_payment2schedule_state(int schedule_id, int state_date, int payment_id){
	sqlite3_stmt *st;
	if(sqlite3_prepare_v2(db,
			"update fv_schedule_state set payment_id=?"
			" where schedule_id=? and state_date=?",
			-1, &st, 0)!=SQLITE_OK){
		fprintf(stderr, "%s error at sqlite3_prepare_v2 %s\n", __PRETTY_FUNCTION__, sqlite3_errmsg(db));
		return -1;
	}
	sqlite3_bind_int(st, 1, payment_id);
	sqlite3_bind_int(st, 2, schedule_id);
	sqlite3_bind_int(st, 3, state_date);
	return step_and_finalize(st);	
}
