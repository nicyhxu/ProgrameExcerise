#include <stdio.h>
#include <time.h>
#include <sys/times.h>
#include <sys/types.h>
#include <unistd.h>

#define test_accuracy
#define MILLION 1000000
#define WAIT for(i=0;i<298765432;i++);
static void do_cmd(char *);
static void pr_times(clock_t, struct tms *, struct tms *);

#ifdef test_times
static void do_cmd(char *cmd){
        struct tms tmsstart, tmsend;  
        clock_t start, end;
        int status;
        if((start=times(&tmsstart))== -1)
                puts("times error");
        if((status=system(cmd))<0)
                puts("system error");
        if((end=times(&tmsend))== -1)
                puts("times error");
        pr_times(end-start, &tmsstart, &tmsend);
        exit(0);
}
static void pr_times(clock_t real, struct tms *tmsstart, struct tms *tmsend){
        static long clktck=0;
        if(0 == clktck)
                if((clktck=sysconf(_SC_CLK_TCK))<0)
                           puts("sysconf err");
        printf("real:%7.2f\n", real/(double)clktck);
        printf("user-cpu:%7.2f\n", (tmsend->tms_utime - tmsstart->tms_utime)/(double)clktck);
        printf("system-cpu:%7.2f\n", (tmsend->tms_stime - tmsstart->tms_stime)/(double)clktck);
        printf("child-user-cpu:%7.2f\n", (tmsend->tms_cutime - tmsstart->tms_cutime)/(double)clktck);
        printf("child-system-cpu:%7.2f\n", (tmsend->tms_cstime - tmsstart->tms_cstime)/(double)clktck);
}
#endif

#if !defined (_POSIX_TIMERS)
# define _POSIX_TIMERS (-1)
#endif
#if !defined (_POSIX_CLOCK_SELECTION)
/* Clock selection was defined in 2001 and became mandatory in 2008. */
# define _POSIX_CLOCK_SELECTION (-1)
#endif
#if !defined (_POSIX_MONOTONIC_CLOCK)
# define _POSIX_MONOTONIC_CLOCK (-1)
#endif

#if (_POSIX_TIMERS > 0)
static unsigned vlc_clock_prec;

# if (_POSIX_MONOTONIC_CLOCK > 0) && (_POSIX_CLOCK_SELECTION > 0)
/* Compile-time POSIX monotonic clock support */
#  define extPlayer_clock_id (CLOCK_MONOTONIC)

# elif (_POSIX_MONOTONIC_CLOCK == 0) && (_POSIX_CLOCK_SELECTION > 0)
/* Run-time POSIX monotonic clock support (see clock_setup() below) */
static clockid_t extPlayer_clock_id;

# else
/* No POSIX monotonic clock support */
#   define extPlayer_clock_id (CLOCK_REALTIME)
#   warning Monotonic clock not available. Expect timing issues.

# endif /* _POSIX_MONOTONIC_CLOKC */
#endif /*_POSIX_TIMERS*/

int add (int a,int b) {
	int c = a+b;
	return c;
}
int main(int argc, char *argv[])
{
#ifdef clock
	long val = 0;
	char text[] = "abcdefghijklmno";
	int i = 0;
	int c = add(1,2);
	text[12] = '\0';// m被清除
	while(text[++i] != '\0')// 呈现隔一出一的趋势，1,3,5...
	{
		printf("%c ,", text[i++]);
		printf("text[++i]=%d \n",text[++i]);
	}
	if((val=sysconf(_SC_TIMERS))<0){
		printf( "_SC_TIMERS not support");
	}
#if (_POSIX_TIMERS > 0)
   long ii=1000L;
   clock_t start, finish;
   double  duration;
   printf( "Time to do %ld empty loops is ", ii );
   start = clock();
   while (--ii){
    system("cd");
   }
   finish = clock();
   duration = (double)(finish - start) / CLOCKS_PER_SEC;
   printf( "%f seconds\n", duration );
#endif
#endif
#ifdef test_times
   int i = 0;
   for(i=1; argv[i]!=NULL; i++){
    do_cmd(argv[i]);
   }
   exit(1);
#endif
#ifdef test_clock_gettime
   long int loop = 1000;
   struct timespec tpstart;
   struct timespec tpend;
   long timedif;

   clock_gettime(extPlayer_clock_id, &tpstart);

   while (--loop){
    system("cd");
   }

   clock_gettime(extPlayer_clock_id, &tpend);
   timedif = MILLION*(tpend.tv_sec-tpstart.tv_sec)+(tpend.tv_nsec-tpstart.tv_nsec)/1000;
   fprintf(stdout, "it took %ld microseconds \n", timedif);
   fprintf(stdout, "this_clock_id =%ld\n", extPlayer_clock_id);
#endif
#ifdef test_gettimeofday
   int i=10000000;
   struct timeval tvs,tve;
   gettimeofday(&tvs,NULL);
   while (--i);
   gettimeofday(&tve,NULL);
   double span = tve.tv_sec-tvs.tv_sec + (tve.tv_usec-tvs.tv_usec)/1000000.0;
   printf("time: %.12f\n",span);
#endif
#ifdef test_accuracy
    int i;
    long ttt;
    clock_t s,e;
    struct tms aaa;
    s=clock();
    WAIT;
    e=clock();
    printf("clock time : %.12f\n",(e-s)/(double)CLOCKS_PER_SEC);


    long tps = sysconf(_SC_CLK_TCK);
    s=times(&aaa);
    WAIT;
    e=times(&aaa);
    printf("times time : %.12f\n",(e-s)/(double)tps);


    struct timeval tvs,tve;
    gettimeofday(&tvs,NULL);
    WAIT;
    gettimeofday(&tve,NULL);
    double span = tve.tv_sec-tvs.tv_sec + (tve.tv_usec-tvs.tv_usec)/1000000.0;
    printf("gettimeofday time: %.12f\n",span);


    struct timespec tpstart;
    struct timespec tpend;

    clock_gettime(CLOCK_REALTIME, &tpstart);
    WAIT;
    clock_gettime(CLOCK_REALTIME, &tpend);
    double timedif = (tpend.tv_sec-tpstart.tv_sec)+(tpend.tv_nsec-tpstart.tv_nsec)/1000000000.0;
    printf("clock_gettime time: %.12f\n", timedif);
#endif
   return 0;
}
/*c99
jichu5.cpp(13) : error C2105: '++' needs l-value
sizeof 编译就知道数组名表示的是数组对象;包含数组的长度
*/
