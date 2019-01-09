#include "PaymentProcessing.h"
#include "julianday.h"
#include <stdio.h>  
#include <string>   
#include <math.h>
#include <vector>

using namespace std;
//~ 01.03.2013|3273,97|73,97|1800|
//~ 01.04.2013|1510,12|36,15|1800|
//~ 01.05.2013|0|16,14|1526,26|

bool getCSVdate(FILE*f, int* state_date){
	int c, d,m,y;	
	if((c = fgetc(f)) == EOF)
		return true;
	if((c<'0')||(c>'9'))
		return false;
	d = (c-'0')*10;
	if((c = fgetc(f)) == EOF)
		return false;
	if((c<'0')||(c>'9'))
		return false;
	d+=c-'0';
	
	if((c = fgetc(f)) == EOF)
		return false;
	if(c!='.')
		return false;	

	if((c = fgetc(f)) == EOF)
		return false;
	if((c<'0')||(c>'9'))
		return false;
	m = (c-'0')*10;
	if((c = fgetc(f)) == EOF)
		return false;
	if((c<'0')||(c>'9'))
		return false;
	m+=c-'0';

	
	if((c = fgetc(f)) == EOF)
		return false;
	if(c!='.')
		return false;	
		
	if((c = fgetc(f)) == EOF)
		return false;
	if((c<'0')||(c>'9'))
		return false;
	y = (c-'0')*1000;
	if((c = fgetc(f)) == EOF)
		return false;
	if((c<'0')||(c>'9'))
		return false;
	y += (c-'0')*100;
	if((c = fgetc(f)) == EOF)
		return false;
	if((c<'0')||(c>'9'))
		return false;
	y += (c-'0')*10;
	if((c = fgetc(f)) == EOF)
		return false;
	if((c<'0')||(c>'9'))
		return false;
	y+=c-'0';		
	
	*state_date = computeJD(d, m, y);
	if((c = fgetc(f)) == EOF)
		return false;
	if(c!='|')
		return false;	
				
	return true;
}

bool getCSVfinancial(FILE*f, financial* d, string *err){
	int c;
	string s;
	while((c = fgetc(f)) != '|'){
		if(c == EOF){
			*err="Неожиданный конец файла";
			return false;
		}
		if(((c>='0')&&(c<='9')) || (c==',') || (c=='.'))
			s+=c;
		else{
			*err="Недопустимый символ '";
			*err+=c;
			*err+="'";
			return false;
		}
	}
	if(str2financial(s.c_str(), d)<0){
		*err="Ошибка преобразования";
		return false;
	}
	return true;
}

bool getCSVcomment(FILE*f, string* comment){
	int c;
	comment->clear();
	while((c = fgetc(f)) != '\n'){
		if(c == EOF)
			return true;
		*comment += c;
	}
	return true;
}

std::string getCSVline(FILE*f, int* state_date, financial* remainder, financial* interest, financial* payment,string* comment){
	if(feof(f))
		return "eof";
	*state_date=0;
	if(!getCSVdate(f, state_date))
		return "Ошибка загрузки даты";		
	if(!(*state_date))
		return "eof";
	string err;
	if(!getCSVfinancial(f, remainder, &err))
		return "Ошибка загрузки остатка: "+err;
	if(!getCSVfinancial(f, interest, &err))
		return "Ошибка загрузки суммы процентов: "+err;
	if(!getCSVfinancial(f, payment, &err))
		return "Ошибка загрузки суммы платежа: "+err;
	if(!getCSVcomment(f, comment))
		return "Ошибка загрузки комментария";
	return "";
}

string PaymentProcessing::loadStatesFromCSV(int schedule_id, const char* fn){
	FILE*f =fopen(fn,"r");
	int state_date;
	financial remainder, interest, payment;
	string comment;
	string r;
	while((r = getCSVline(f, &state_date, &remainder, &interest, &payment, &comment))==""){
		db->insert_fv_schedule_state(schedule_id, state_date, remainder, interest, payment, comment.c_str(), 0);		
	}
	fclose(f);
	if(r=="eof")
		r="";
	return r;
}


double PaymentProcessing::rateOnDate(int date, int type){
	double r = 0;	
	if(ga4date[type])
		for (int i = 0; i < ga4date[type]->len; i++){
			int start_date = g_array_index (ga4date[type], int, i);
			if( start_date <= date )
				r = g_array_index (ga4value[type], double, i);
			if( start_date >= date )
				break;
		}
	return r;	
}

transparentVar PaymentProcessing::getRateOnPeriod(int dateB, int dateE, int type){
//Учесть, что dateB и dateE могут быть в годах с разным кол-вом дней
// или между ними могут быть годы с разным кол-вом дней
	transparentVar ret;
	double rate0 = rateOnDate(dateB, type);
	int date0 = dateB;
	double rate1;
	int date1;
	
	if(ga4date[type])
		for (int i = 0; i < ga4date[type]->len; i++){
			int start_date = g_array_index (ga4date[type], int, i);
			if( start_date <= dateB )
				continue;
			if( start_date > dateE )
				break;
			
			date1 = start_date;
			rate1 = g_array_index (ga4value[type], double, i);
			
			transparentVar	t_date1(date1);
			transparentVar	t_date0(date0);
			transparentVar	t_date_diff = t_date1 - t_date0;
			t_date_diff.skob();
			transparentVar	t_rate0(rate0);
			
			ret = ret + t_rate0*t_date_diff;
			date0 = date1;
			rate0 = rate1;		
		}
	
	transparentVar	t_dateE(dateE);
	transparentVar	t_date01(date0-1);
	transparentVar	t_date_diff = t_dateE - t_date01;
	t_date_diff.skob();
	transparentVar	t_rate0(rate0);
	return (ret + t_rate0*t_date_diff)/get_days_in_year(dateE)/100;	
}

financial PaymentProcessing::getAnnValue(financial sum, int dateB, int dateE, bool first_month_pay){
    if(dateE <= dateB)
        return 0;
	
	int v_i_date = dateE;
    double v_k_product = 1;
    double v_k_sum = 0;     
	int v_first_pay_date = (first_month_pay)?last_day(dateB):last_day(add_months(dateB, 1));
    
    for(;;v_i_date=first_day(v_i_date)-1){
       int v_pp0_date = first_day(v_i_date);
       if (v_i_date<=v_first_pay_date){
            double v_ps1=getRateOnPeriod(dateB + 1, v_first_pay_date, 0 ).dbl();
            double v_k_sum_t=v_k_sum+v_k_product;
            double v_k_product_t=v_k_product*(1+v_ps1);
            if (v_ps1 < v_k_product_t/v_k_sum_t){
                v_k_sum=v_k_sum_t;
                v_k_product=v_k_product_t;
            }
            break;
        }
        v_k_sum+=v_k_product;
        v_k_product*=(1+getRateOnPeriod(v_pp0_date, v_i_date, 0).dbl());
	}
	
	return llrint(sum*v_k_product/v_k_sum/100)*100+100;    
}

financial PaymentProcessing::getInterestBase(financial rem, int dateB, int dateE, std::string *s, int type){
	transparentVar tRem (rem);
	transparentVar tRateOnPeriod = getRateOnPeriod(dateB, dateE, type);
	tRateOnPeriod.skob();
	transparentVar tRes = tRem * tRateOnPeriod;
	//~ transparentVar tRes = operator*(tRem, tRateOnPeriod);
	tRes.round2();
	if(s)
		*s = tRes.str();
	return tRes.fin();
}

financial PaymentProcessing::getOverDueInterest(financial rem, int dateB, int dateE, string *s){
	return getInterestBase(rem, dateB, dateE, s, 1);
}

financial PaymentProcessing::getInterest(financial rem, int dateB, int dateE, string *s){
	return getInterestBase(rem, dateB, dateE, s, 0);
}

financial PaymentProcessing::getAllDolg(int date, std::string *proc){
	int rem_date;
	financial rem;
	int overdue_date;
	financial overdue;
	db->state_after_last_movements(agr_id, date, &rem_date, &rem, &overdue_date, &overdue);
	if(rem+overdue==0){
		if(proc)
			proc->clear();
		return 0;
	}

	if(rem>0){
		rem+=getInterest(rem, rem_date+1, date, proc);
	}		
	if(overdue>0){
		string s_overdue;
		overdue+=getOverDueInterest(overdue, overdue_date+1, date, (proc)?(&s_overdue):0);
		if(proc){
			if(!proc->empty())
				*proc+=" + ";
			*proc+=s_overdue;
		}
	}
	return rem+overdue;
}


string PaymentProcessing::createAnnScheduleBase(int schedule_id, financial sum, financial v_ann_plat, int dateB, int dateE, bool first_month_pay){
    int v_pp0_date = dateB+1; //Дата начала платежного периода. Проценты начисляются за день, если на его утро был остаток ссуды
    int v_i_date = last_day(dateB);	//Дата платежа
	if (!first_month_pay /*Платеж в следующем месяце*/ 
		|| v_i_date<=v_pp0_date /*Бывает в конце месяца. Не логично платить в дату выдачи*/){
		v_i_date=last_day(v_i_date+1);             
	}
    
	for(financial v_rem = sum;;){
		string s;
        if (v_i_date>=dateE) {
            financial v_proc=getInterest(v_rem, v_pp0_date, dateE, &s);
            db->insert_fv_schedule_state(schedule_id, dateE, 0, v_proc, v_rem+v_proc, s.c_str(),0);
            break;
        }
        financial v_proc=getInterest(v_rem, v_pp0_date, v_i_date, &s);
        v_rem+=v_proc-v_ann_plat; 
        db->insert_fv_schedule_state(schedule_id, v_i_date, v_rem, v_proc, max(v_ann_plat,v_proc), s.c_str(),0);
        
        v_pp0_date=v_i_date+1;
        v_i_date=last_day(v_pp0_date);
    }   
    return "";       
}

string PaymentProcessing::create1PaymentSchedule(int schedule_id, financial sum, int dateB, int dateE){
	string comment;
	financial v_proc = getInterest(sum, dateB+1, dateE, &comment);
	return (db->insert_fv_schedule_state(schedule_id, dateE, 0, v_proc, sum+v_proc, comment.c_str(), 0)==0)?"":"Ошибка вставки последней строки графика";	 
}


string PaymentProcessing::createPaymentVyd(financial sum, financial v_ann_plat, int dateB, int dateE, bool first_month_pay, bool one_payment, const char* comment, const char* fn){
	if(!fn){
		if (dateB >= dateE)
			return "Дата начала больше либо равна дате окончания";
	}
	
	int payment_id = db->insert_fv_payment(agr_id, dateB, -sum, 0, 0, 0, comment);
	if(payment_id<0)
		return "Ошибка вставки платежа";
	
	int schedule_id = db->insert_fv_schedule(payment_id, 1 /*Выдача*/);
	if(schedule_id < 0)
		return "Ошибка вставки графика";
		
	if(db->insert_fv_schedule_state(schedule_id, dateB, sum, 0, -sum, "", schedule_id)!=0)
		return "Ошибка вставки первой строки графика";
		
	if(fn)
		return loadStatesFromCSV(schedule_id, fn);
		
	if( one_payment )
		return create1PaymentSchedule(schedule_id, sum, dateB, dateE);
		
	return createAnnScheduleBase(schedule_id, sum, v_ann_plat, dateB, dateE, first_month_pay);
}


typedef struct _Sched{
	int state_date; 
	financial remainder;
	financial payment;
} Sched;

int cb_fv_schedule_state(void* param, int id, PARAMS_fv_schedule_state){ 
	vector<Sched>* v = (vector<Sched>*)param;	
	Sched s;
	s.state_date = state_date;
	s.remainder = remainder;
	s.payment = payment;
	v->push_back(s);
	return 0;
}


string PaymentProcessing::createPayment(financial sum, int date, const char* comment, const char* fn){
	financial allDolg = getAllDolg(date);
	if(sum > allDolg)
		return "Сумма платежа ("+financial2str(sum)+") больше, чем долг по договору ("+financial2str(allDolg)+")";
	
	int rem_date;
	financial rem;
	int overdue_date;
	financial overdue;
	db->state_after_last_movements(agr_id, 2000000000, &rem_date, &rem, &overdue_date, &overdue);
	
	PaymentProcessing s(agr_id, db);
	
	if (sum < 0) {
		if(overdue>0)
			return "Выдача нового транша при просрочке невозможна!";
		//Сделать проверку на то, что договор с одним платежом
		int plan_end_date = db->getPlanEndDate(agr_id);
		if(date > plan_end_date)
			return "Дата выдачи нового транша не может быть больше даты окончания договора!";
		int payment_id = db->insert_fv_payment(agr_id, date, sum, 0, 0, 0, comment);
		int schedule_id = db->insert_fv_schedule(payment_id, 1 /*Выдача - сделать константы*/);
		if(schedule_id < 0)
			return "Ошибка вставки графика";
		financial interest = s.getInterest(rem, rem_date+1, date, 0);
		rem = rem + interest - sum;
		db->insert_fv_schedule_state(payment_id, date, rem, interest, sum, "", payment_id);
		string str;
		interest = s.getInterest(rem, date+1, plan_end_date, &str);
		db->insert_fv_schedule_state(payment_id, plan_end_date, 0, interest, rem+interest, str.c_str(), 0);
		return "";
	}
	//~ printf("%i %g %i %g\n", rem_date, rem, overdue_date, overdue);

//Вынос на просрочку платежей между датой погашения и датой последнего платежа	
	vector<SchedPayment> vp;	
	db->getMissedPayments(agr_id, date, &vp);
	
	for(vector<SchedPayment>::const_iterator i=vp.begin();i!=vp.end();i++){
		//~ printf("%i %i %g\n", i->date, i->schedule_id, i->value);
		financial overdue_interest=s.getOverDueInterest(overdue, overdue_date+1, i->date, 0);
		overdue=overdue+overdue_interest+i->value;
		int payment_id = db->insert_fv_payment(agr_id, i->date, 0, overdue, overdue_interest, -i->value, "Автоматический вынос на просрочку");
		db->bind_payment2schedule_state(i->schedule_id, i->date, payment_id);
		overdue_date = i->date;
	}
	
	SchedPayment SP;
	db->getSchedPaymentOnDate(agr_id, date, &SP);
	
	financial overSched = sum - SP.value; //Сколько внесено сверх планового платежа
//Пересчет просрочки	
	financial overdue_interest=s.getOverDueInterest(overdue, overdue_date+1, date, 0);		
	overdue+=overdue_interest; 		
	financial overdue_payment=min(overdue, overSched);
	overdue-=overdue_payment;
	
	financial overSched2Sched = overSched-overdue_payment; //Сколько из внесенного сверх планового платежа учтется как досрочное погашение
	//Округление - вынужденная мера из-за погрешности представления чисел типом данных double - теперь не надо, так как ФЗ
	int payment_id = db->insert_fv_payment(agr_id, date, sum, overdue, overdue_interest, overdue_payment, comment);
	if(fn){
		int schedule_id = db->insert_fv_schedule(payment_id, 2 /*Погашение*/);
		if(schedule_id < 0)
			return "Ошибка вставки графика";		
		string r = loadStatesFromCSV(schedule_id, fn);
		db->bind_payment2schedule_state(schedule_id, date, payment_id); 
		return r;
	}
	if(overSched2Sched==0){
		if(SP.date)
			db->bind_payment2schedule_state(SP.schedule_id, SP.date, payment_id); 
	}else{ 
		int last_sched_id = db->getLastScheduleID(agr_id);
		vector<Sched> v_prev_schedule;
		db->select_fv_schedule_state(last_sched_id, cb_fv_schedule_state, &v_prev_schedule);
		
		db->insert_fv_schedule(payment_id, 2);	//В счет следующего платежа
		financial payment = overSched2Sched + ((SP.date!=0)?SP.value:0);
		financial interest = s.getInterest(rem, rem_date+1, date, 0);
		rem = rem + interest - payment;
		db->insert_fv_schedule_state(payment_id, date, rem, interest, payment, "", payment_id);
		if(rem>0)
			for (vector<Sched>::const_iterator i = v_prev_schedule.begin();i!=v_prev_schedule.end();i++){
				if(i->state_date > date){
					financial diffpayment = min(i->payment, overSched2Sched);
					if(diffpayment == i->payment){
						overSched2Sched-=i->payment;
					}else{
						overSched2Sched=0;
						financial payment = i->payment - diffpayment;
						string str;
						financial interest = s.getInterest(rem, date+1, i->state_date, &str);
						rem = rem + interest - payment;
						if(rem>0)
							db->insert_fv_schedule_state(payment_id, i->state_date, rem, interest, payment, str.c_str(), 0);
						else{
							db->insert_fv_schedule_state(payment_id, i->state_date, 0, interest, rem+payment, str.c_str(), 0);
							break;
						}
						date = i->state_date;
					}
				}
			}
	}
	return "";
}

int PaymentProcessing::callback_fv_rate(void* param, PARAMS_ID_fv_rate, PARAMS_fv_rate){
	PaymentProcessing* s = (PaymentProcessing*)param;		
	g_array_append_val (s->ga4date[type-1], id);
	g_array_append_val (s->ga4value[type-1], value);	
}

PaymentProcessing::PaymentProcessing(int _agr_id, DB* _db):db(_db),agr_id(_agr_id){
	if(agr_id){
		ga4date[0] = g_array_new (FALSE, FALSE, sizeof (int));
		ga4value[0] = g_array_new (FALSE, FALSE, sizeof (double));
		ga4date[1] = g_array_new (FALSE, FALSE, sizeof (int));
		ga4value[1] = g_array_new (FALSE, FALSE, sizeof (double));
		db->select_fv_rate(agr_id, callback_fv_rate, this);
	}else{
		ga4date[0] = ga4value[0] = ga4date[1] = ga4value[1] = 0;
	}
}

PaymentProcessing::~PaymentProcessing(){
	if(ga4date[0])
		g_array_free (ga4date[0], TRUE);
	if(ga4value[0])
		g_array_free (ga4value[0], TRUE);
	if(ga4date[1])
		g_array_free (ga4date[1], TRUE);
	if(ga4value[1])
		g_array_free (ga4value[1], TRUE);
}
