#include <stdio.h>

int __attribute__((stdcall)) add(int a, int b)
{
    return a+b;
}
int main()
{
	int a = 1;
	int b = 2;
	add(a,b);
	return 1;
}