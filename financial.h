#include <string>

#ifndef FINANCIAL_H
#define FINANCIAL_H

#define financial long long

//~ #define DOUBLE2STR(NAME) char str_##NAME[20]; snprintf(str_##NAME , sizeof(str_##NAME),"%2.2f", NAME);

int str2financial(const char* str, financial* f);
std::string financial2str(financial f);

#endif
