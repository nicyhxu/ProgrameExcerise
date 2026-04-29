#include <stdio.h>
#include <iostream>
using namespace std;
#define __VLC_SYMBOL( symbol  ) CONCATENATE( symbol, MODULE_SYMBOL )
#define CONCATENATE( y, z ) CRUDE_HACK( y, z )
#define CRUDE_HACK( y, z )  y##__##z
# define MODULE_SYMBOL 2_1_0a

#define tmpfile "E:/tftp/a.txt"
int main()
{
  int c[2]={100,200};
  int a=(long)c;
  int*b;
  printf("%d \n",a);
  b=&a;
  b=(int*)a;   //指针指向了a表示的地址;
  *b=a;
  //char*ast = __VLC_SYMBOL(user);
  int e[][3]  = {6,5,4,3,2,1,0};
  int (*p)[3] = e; //p是一个二级指针，p[0]是一级指针;
  cout<<p[0][0]<<*(p[0]+1)<<(*p)[2]<<endl;
  cout<<p[0][0]<<**(p+1)<<(*p)[2]<<endl; //*p是一级指针;
  int ac[6] = {1,2,3,4,5,6};
  FILE * pfile = NULL;
  pfile = fopen(tmpfile,"w+b");
  for(int i=0;i<6;i++)
   fwrite(&ac[i],sizeof(int),1,pfile);
  rewind(pfile);
  int read1,read2;
  fread(&read1,sizeof(int),1,pfile);
  fread(&read2,sizeof(int),1,pfile);
  cout<<read1<<read2<<endl;
  fclose(pfile);
}
/*c99
涉及到整数转换为指针
一般有两种情况会用到：
   句柄是一种指向指针的,但是句柄里面的值是不会变化的;
   线程函数参数;
runing error:
jichu:10 is ok when a is a vaild address;
这里编译时没有问题的,整数转换成指针;这里的整数其实是地址;
*/