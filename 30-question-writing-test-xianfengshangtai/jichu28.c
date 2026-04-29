#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <execinfo.h>
#include <signal.h>

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

void dump_stack(void)
{
    void *array[30];
 	size_t size;
	char **strings;
	size_t i;
	size = backtrace(array,ARRAY_SIZE(array));  //返回的是实际的调用栈的指针大小fp(arm)；
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
	exit(EXIT_SUCCESS);
}

void sighandler_dump_stack(int sig)
{
	psignal(sig,"jichu28");     //主要是打印出进程名称;
	dump_stack();
	signal(sig,SIG_DFL);
	raise(sig);
}

void func_c()
{
	*((volatile int*)0x0) = 0x9999;
}

void func_b()
{
	func_c();
}

void func_a()
{
	func_b();
}

int main(int argc,const char *argv[])
{
	if(signal(SIGSEGV,sighandler_dump_stack) == SIG_ERR)
		perror("cannot catch SIGEGV");
	
	func_a();
	return 0;
}


