//~ #include <string>
//~ #include <stdlib.h>
#include "financial.h"

using namespace std;

//~ int str2double(const char* str, double *d){
	//~ string s;
	//~ bool f=false;
	//~ for(const char* pc=str;*pc;pc++)
		//~ if(*pc>='0' && *pc<='9')
			//~ s+=*pc;
		//~ else if (*pc==',' || *pc=='.'){
			//~ if(f)
				//~ return -1;
			//~ f=true;
			//~ s+=',';
		//~ }else
			//~ return -2;
	//~ if(s.empty())
		//~ return -3;
	//~ *d = atof(s.c_str());
	//~ return 0;
//~ }

int str2financial(const char* str, financial* v){
	bool f=false;
	int dop=2;
	*v = 0;
	if(!(*str))
		return -3;
	const char* pc=str;
	bool signFlag;
	if(signFlag=(*pc=='-'))
		pc++;
	for(;*pc;pc++)
		if(*pc>='0' && *pc<='9'){
			*v=*v*10+*pc-'0';
			if(f)
				if((--dop)==0)
					break;
		}else if (*pc==',' || *pc=='.'){
			if(f)
				return -1;
			f=true;
		}else
			return -2;
	while(dop--)
		*v*=10;
	if(signFlag)
		*v=-*v;
	return 0;	
}

std::string financial2str(financial f){
	char buf[64];
	int bufPos=-1;	
	bool signFlag;
	if(signFlag=(f<0))
		f=-f;
	for(int i=0;(i<3) || f;i++,f/=10){
		char c = f%10;		
		buf[++bufPos]=c+'0';
	};
	string str;
	if(signFlag)
		str+='-';
	for(;bufPos>=0;bufPos--){
		if (bufPos==2-1)
			str+='.';
		str+=buf[bufPos];
	}
	return str;
}
