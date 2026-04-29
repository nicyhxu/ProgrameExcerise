#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <iostream>
#include <vector>
using namespace std;

int cstring_cmp(const void*a,const void*b)
{
	const char**ia=(const char**)a;
	const char**ib=(const char**)b;
	return strcmp(*ia,*ib);
}


int comlen(char*p,char*q)
{
	int length=0;
	while(*p&&*q)
	{
		if(*p==*q)  
		{  
			p++;
			q++;
			length++;
		}
		else
			return length;
	}
	return length;
}
int main()
{ 
   char pch;
	char quhar[1024];
	char *pChar[1024];
	char *temp = NULL;
	int n=0;
	int icount = 0;
	while((pch=getchar()) != '\n')
	{
		/*if(pch >='1' && pch<='9')*/
		{
			pChar[n]=(&quhar[n]);
			quhar[n++]=pch;
		}
	}
	quhar[n]='\0';
	temp = &quhar[0];
	qsort(pChar, n, sizeof(char*),cstring_cmp);
	int len=0;
	int maxi=0;
	int maxlen=0;
	for(int i=0;i<n-1;i++)
	{
		len=comlen(pChar[i],pChar[i+1]);  //能够找到起始，最大长度和最小长度的起始的差值应该等于长度;
		if(len>maxlen && abs(pChar[i+1] - pChar[i]) >= len)
		{
			maxlen=len;
			maxi=i;
			icount = 0;
		}
		if(len == maxlen)
		{
			maxlen=len;
			maxi=i;
			icount ++;
		}
	}
    if(icount <1 || maxlen <=1)
	{
		cout<<"\""<<"Input error!"<<"\""<<endl;
		return false;
	}
    char ch_tmp;
	cout<<"\"";
	for (int i = 0; i < maxlen; i++)
	{
		ch_tmp = *(pChar[maxi] + i);
		cout<<ch_tmp;
	}
	cout<<"\"";
   return 0;
}