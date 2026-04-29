/*  char *strstr(const char *haystack, const char *needle)
*********** 标准库函数,C库函数中strstr在字符串haystack中查找第一次出现字符串needle的位置
不包含终止符'\0'
**********
*/
#include <stdio.h>
#include <string.h>

int main()
{
	const char haystack[20] = "RUNOOB";
	const char needle[10] = "NOOB";
	const char * ret;
	ret = strstr(haystack, needle);
	printf("子字符串是： %d\n", ret - &haystack[0]);
	return 0;
}



