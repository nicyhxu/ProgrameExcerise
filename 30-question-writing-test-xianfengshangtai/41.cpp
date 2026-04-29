#include <stdio.h>
#include <string.h>
#include<iostream>
using namespace std;

#define MAX 1000000 
typedef long long ll;
char s[MAX];
ll d[MAX][4] = {0};

int main()
{
	scanf("%s",s+1);
	int n=strlen(s+1);   //获取字符的长度;
	d[0][0] = 1;
	for(int i=1;i<=n;i++)
	{
		for(int j=0;j<=2;j++)
		{
			if(j<2) 
			   d[i][j+1] += d[i-1][j];
			d[i][j] += d[i-1][j];  //类似于赋值,表示包含自己不去掉;
			//从0到i中间如果有重复的，那么对应的删除1自己的那部分需要减掉;
			for(int k=i-1; k>1 && i-k<=j;k--)
			{
				if(s[k] == s[i]) 
				{ 
				  d[i][j] -= d[k-1][j-(i-k)];   //删除前面重复的个数；
				  cout<<"chongfu"<<k<<i<<i-k<<j<<endl;
				  break; 
				} 
			}
		}
		//如果有重复的;
	}
    for(int i=0;i<=n;i++)
	{
		for(int j=0;j<3;j++)
		   cout<<d[i][j];
	   cout<<endl;
	}
}