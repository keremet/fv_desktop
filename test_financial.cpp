#include "financial.h"

using namespace std;

int main(int agrc, char** agrv){
	printf("%s\n", financial2str(1002).c_str());
	financial v;
	const char* str="1.001";
	int e = str2financial(str, &v);
	printf("%i %i\n", e, v);
	return 0;
}
