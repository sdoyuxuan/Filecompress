#include "filecompress.h"
void test()
{
	//fopen("2.txt", "w");
	Filecompress Fc;
	//Fc.compress("2.txt");
	Fc.uncompress("2.hzp");
}
int main()
{
	test();
	return 0;
}
