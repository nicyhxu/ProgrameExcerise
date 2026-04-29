#include <stdio.h>

int add (int a,int b) {
	int c = a+b;
	return c;
}
int main()
{
	char text[] = "abcdefghijklmno";
	int i = 0;
	int c = add(1,2);
	text[12] = '\0';// m被清除
	while(text[++i] != '\0')// 呈现隔一出一的趋势，1,3,5...
	{
		printf("%c ,", text[i++]);
		printf("text[++i]=%d \n",text[++i]);
	}
	return 1;
}
/*c99
jichu5.cpp(13) : error C2105: '++' needs l-value
sizeof 编译就知道数组名表示的是数组对象;包含数组的长度

*/