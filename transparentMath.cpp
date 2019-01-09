#include <string>
#include <math.h>
#include <stdio.h>
#include "julianday.h"
#include "financial.h"
#include "transparentMath.h"
using namespace std;

transparentVar::transparentVar(double _d):d(_d){
	//~ DOUBLE2STR(d);
	//~ s = str_d;
	s = financial2str((financial)(d*100));
}

transparentVar::transparentVar(financial _f):d(double(_f)){
	//~ DOUBLE2STR(d);
	//~ s = str_d;
	s = financial2str(_f);
}

transparentVar::transparentVar(int i):d(i){
	char s_date[2+1+2+1+4 + 1];
	JD2str(i, s_date);
	s = string("'")+s_date+"'";
}

transparentVar& transparentVar::operator=(transparentVar &tv){
	s = tv.s;
	d = tv.d;
	return *this;
}

string transparentVar::skob(){
	s = string("(")+s+")";
	return s;
}

void transparentVar::round2(){
	//~ d = round(d*100)/100;
	s = string("ОКРУГЛЕНИЕ(")+s+")";
}

transparentVar& operator+(transparentVar &tv1, transparentVar &tv2){
	if (tv1.s.empty()){
		tv1.d=tv2.d;
		tv1.s=tv2.s;
	}else if (!tv2.s.empty()){
		tv1.d+=tv2.d;
		tv1.s+='+';
		tv1.s+=tv2.s;
	}
	return tv1;
}
transparentVar& operator-(transparentVar &tv1, transparentVar &tv2){
	tv1.d-=tv2.d;
	tv1.s+='-';
	tv1.s+=tv2.s;
	return tv1;
}
transparentVar& operator*(transparentVar &tv1, transparentVar &tv2){
	tv1.d*=tv2.d;
	tv1.s+='*';
	tv1.s+=tv2.s;
	return tv1;
}
transparentVar& operator/(transparentVar &tv1, int i){
	tv1.d/=i;
	char str_i[20]; snprintf(str_i , sizeof(str_i),"%i", i);	
	tv1.s += '/';
	tv1.s += str_i;
	return tv1;
}


//~ int main(int agrc, char** agrv){
	//~ transparentVar a(10.55);
	//~ transparentVar b(17.55);
	//~ transparentVar c(2.0);
	//~ transparentVar t;
	//~ t = b*c;
	//~ t.skob();
	//~ a=a+t;
	//~ cout << a.dbl()<<' '<< a.str()<<endl;
	//~ return 0;
//~ }
