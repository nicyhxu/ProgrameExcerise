#include <stdio.h>
#define container_of(a,b) ({\
         char *__tmptr = (b);\
        (int*)(__tmptr-a);})

int main()
{
	//int a = 1;
	//int b = 2;
	//add(a,b);
	char * o1 = "abc";
	char * o3 = "dfg";
	int*b = container_of(o3,o1);
	int a[3][4][5] = {0};
	return 1;
}