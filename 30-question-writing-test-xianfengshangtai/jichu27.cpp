#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
/***
   用来测试glibc中的backtrace接口函数;
***/
void print_trace(void)
{
	void *array[10];
	size_t size;
	char **strings;
	size_t i;
	size = backtrace(array,10);  //返回的是实际的调用栈的指针大小fp(arm)；
	strings = backtrace_symbols(array,size);  //内部通过addline函数来获取函数名称;
	if(NULL == strings)
	{
		perror("backtrace_symbols");
		exit(EXIT_FAILURE);
	}
	printf("Obtained %zd stack frames \n");
	for( i =0 ;i< size ;i++)
		printf("%s\n",strings[i]);
	free(strings);
	strings = NULL;
}

void dummy_function(void)
{
	print_trace();
}

int main(int argc,char *argv[])
{
	dummy_function();
}