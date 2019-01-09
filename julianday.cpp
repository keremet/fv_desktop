#include "stdio.h"
#include "stdlib.h"
#include "time.h"

int computeJD(int D, int M, int Y){
	if( M<=2 ){
		Y--;
		M += 12;
	}
	int A = Y/100;
	int  B = 2 - A + (A/4);
	int X1 = 36525*(Y+4716)/100;
	int X2 = 306001*(M+1)/10000;
	return ((X1 + X2 + D + B - 1524/*- 1524.5 */));
}

int JDnow(){
	time_t t = time(0);
	struct tm* TM = localtime(&t);
	return computeJD(TM->tm_mday, TM->tm_mon+1, TM->tm_year+1900);
}

void computeYMD(int JD, int *day, int *month, int *year){
	int Z, A, B, C, D, E, X1;

	Z = JD; //(int)((JD + 43200000)/86400000); == +=0.5 - мы их не вычитаем при формировании JD
	A = (int)((Z - 1867216.25)/36524.25);
	A = Z + 1 + A - (A/4);
	B = A + 1524;
	C = (int)((B - 122.1)/365.25);
	D = (36525*C)/100;
	E = (int)((B-D)/30.6001);
	X1 = (int)(30.6001*E);
	*day = B - D - X1;
	*month = E<14 ? E-1 : E-13;
	*year = (*month)>2 ? C - 4716 : C - 4715;
}

int first_day(int date){
	int day, month, year;
	computeYMD(date,&day,&month,&year);
	return computeJD(1, month, year);
}

int daysInMonth(int month, int year){
	switch(month){
/*		case 1:
		case 3:
		case 5:
		case 7:
		case 8:
		case 10:
		case 12:
			day = 31;
			break;*/
		case 4:
		case 6:
		case 9:
		case 11:
			return 30;
		case 2:
			return (year&0x03)?28:29;
	}
	return 31;
}

int last_day(int date){
	int day, month, year;
	computeYMD(date,&day,&month,&year);
	return computeJD(daysInMonth(month, year), month, year);
}

int add_months(int date, int months /*1 или -1*/){
	int day, month, year;
	computeYMD(date,&day,&month,&year);
	month+=months;
	if(month>12){
		month-=12;
		year++;
	}else if(month<1){
		month+=12;
		year--;
	}
	return computeJD(day<?daysInMonth(month, year), month, year);
}



void JD2str(int JD, char* str){
	if(JD == 0)
		*str = 0;
	else{
		int day, month, year;
		computeYMD(JD, &day, &month, &year);
		snprintf(str, 2+1+2+1+4 + 1, "%02i.%02i.%04i", day, month, year);
	}
}

void JD2full_str(int JD, char* str, int buflen){
	if(JD == 0)
		*str = 0;
	else{
		int day, month, year;
		const char* smonth[]={"января", "февраля", "марта", "апреля", "мая", "июня", "июля", "августа", "сентября", "октября", "ноября", "декабря"};
		computeYMD(JD, &day, &month, &year);
		snprintf(str, buflen, "%02i %s %04i", day, smonth[month-1], year);
	}
}

int str2JD(const char* str){
	return computeJD(atoi(str),atoi(str+2+1),atoi(str+2+1+2+1));
}


int get_days_in_year(int date){
	int day, month, year;
	computeYMD(date,&day,&month,&year);
	return (year&0x03)?365:366;
}
