#ifndef TRANSPARENT_MATH_H
#define TRANSPARENT_MATH_H

#include <string>
#include <math.h>
#include "financial.h"

class transparentVar{
	std::string s;
	double d;
public:
	transparentVar(){}
	transparentVar(double _d);
	transparentVar(financial _f);
	transparentVar(int i);
	transparentVar& operator=(transparentVar &tv);
	void round2();
	std::string skob();
	std::string str(){
		return s;
	}
	double dbl(){
		return d;
	}
	financial fin(){
		return llround(d);
	}
	
	friend transparentVar& operator+(transparentVar &tv1, transparentVar &tv2);
	friend transparentVar& operator-(transparentVar &tv1, transparentVar &tv2);
	friend transparentVar& operator*(transparentVar &tv1, transparentVar &tv2);
	friend transparentVar& operator/(transparentVar &tv1, int i);
};

transparentVar& operator*(transparentVar &tv1, transparentVar &tv2);
transparentVar& operator+(transparentVar &tv1, transparentVar &tv2);
transparentVar& operator-(transparentVar &tv1, transparentVar &tv2);

#endif
