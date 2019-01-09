#include <glib.h>
#include <string>
#include "db.h"
#include "financial.h"
#include "transparentMath.h"

class PaymentProcessing{
	GArray *ga4date[2];
	GArray *ga4value[2];
	DB* db;
	int agr_id;
	double rateOnDate(int date, int type);
	transparentVar getRateOnPeriod(int dateB, int dateE, int type);
	financial getInterestBase(financial rem, int dateB, int dateE, std::string *s, int type);
	std::string createAnnScheduleBase(int schedule_id, financial sum, financial v_ann_plat, int dateB, int dateE, bool first_month_pay);
	std::string create1PaymentSchedule(int schedule_id, financial sum, int dateB, int dateE);
	static int callback_fv_rate(void* param, PARAMS_ID_fv_rate, PARAMS_fv_rate);
	static int callback_fv_schedule_state(void* param, int id, PARAMS_fv_schedule_state);
	std::string loadStatesFromCSV(int schedule_id, const char* fn);		
public:
	PaymentProcessing(int _agr_id, DB* _db);
	~PaymentProcessing();
	financial getAnnValue(financial sum, int dateB, int dateE, bool first_month_pay);
	financial getInterest(financial rem, int dateB, int dateE, std::string *s=0);
	financial getOverDueInterest(financial rem, int dateB, int dateE, std::string *s=0);
	financial getAllDolg(int date, std::string *proc=0);
	std::string createPaymentVyd(financial sum, financial v_ann_plat, int dateB, int dateE, bool first_month_pay, bool one_payment, const char* comment, const char* fn=0);
	std::string createPayment(financial sum, int date, const char* comment, const char* fn=0);
};
