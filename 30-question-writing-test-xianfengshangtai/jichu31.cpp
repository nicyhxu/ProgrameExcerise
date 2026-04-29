#include<stdio.h>
#include<stdlib.h>
#include <string>
#include <iostream>
using namespace std;
int getlowbit(int x)
{
	return x&(-x);
}
#define MAX_TREE_SIZE 100
//如果新增相同那么
//ab s1
//abb s2 = s1 + s
 
//检查str是否和pattern可以匹配;
bool IsMatch(const char * str,const char * pattern)
{
	if(*str == '\0' && *pattern == '\0')
	{
		cout<<"bigo"<<endl;
		return true;
	}
	if(*str != '\0' && *pattern == '\0')
		return false;
	if(*(pattern+1) == '*')
	{
		if(*pattern == *str || (*pattern == '.' &&  str!='\0'))  //首字符串相同;
		{
		  cout<<"di gui"<<endl;
		  return IsMatch(str,pattern+2) || IsMatch(str+1,pattern+2) || IsMatch(str+1,pattern);  //三种情况匹配0个，匹配1个，匹配多个;
		}
		else
		  return IsMatch(str,pattern+2);  //匹配0个;
	}
	//如果不是*,按照正常匹配流程;
	if(*pattern == *str || (*pattern == '.' && str!='\0'))
		return IsMatch(str+1,pattern+1);
	else
	{
		cout<<"bigo false"<<endl;
		return false;
	}
}

//已经知道了长度之后;
bool IsMatch_DP(const char *str,const char *pattern)
{
	int str_len = 0;
	int patt_len = 0;
	str_len = strlen(str);
	patt_len = strlen(pattern);
	int bool_match[100][100] = {0};
	bool_match[0][0] = 1; //bigo
	for(int i=1;i< patt_len;i++)  //这个有点愣，*不可能是第一个;
	{
		if(pattern[i-1] == '*')   //分析i-1的匹配字符为*，那么可以匹配掉str的0或者1或者多个;
			bool_match[0][i] = bool_match[0][i-2];   //是否bigo取决于是否pattern为*
	}
	for(int i=1;i<str_len;i++)    //表示的是第多少个的字符;
	{
		for(int j=1;j<patt_len;j++) //表示匹配的字符串;
		{
			if(pattern[j-1] == '*') 
			{
				if(str[i-1] == pattern[j-2] || pattern[j-2] == '.')
				{
					bool_match[i][j] = bool_match[i][j-2] || bool_match[i][j-2] || bool_match[i-1][j];
				}
				else
				{
					bool_match[i][j] = bool_match[i][j-2]; //前面一个不相同那么就匹配的是0个;
				}
			}
			if(str[i-1] == pattern[j-1] || pattern[j-1] == '.')
			{
				bool_match[i][j] = bool_match[i-1][j-1];
			}
		}
	}
	return bool_match[str_len - 1][patt_len - 1];
}

/**
    通配符匹配;实现一个？和*的通配符匹配
**/
bool IsMatch_single(const char *str,const char *pattern)
{
	int str_len = 0;
	int patt_len = 0;
	str_len = strlen(str);
	patt_len = strlen(pattern);
	int bool_match[100][100] = {0};
	bool_match[0][0] = 1; //bigo
	for(int i=0;i<str_len;i++)    //表示的是第多少个的字符;
	{
		for(int j=1;j<patt_len;j++) //表示匹配的字符串;
		{
			if(pattern[j-1] == '*') 
			{
				bool_match[i][j] = bool_match[i][j-1] || (i >0 && (bool_match[i-1][j-1] || bool_match[i-1][j]));
			}
			if( i>1 && pattern[j-1] == '？' || pattern[j-1] == str[i-1])
			{
				bool_match[i][j] = bool_match[i-1][j-1];
			}
		}
	}
	return bool_match[str_len - 1][patt_len - 1];
}

bool IsInterleave(const char *s1,const char *s2,const char *s3)
{ 
    if(*s1 == '\0')
	{
	   return !strcmp(s2,s3);
	}
	if(*s2 == '\0')
	{
	   return !strcmp(s2,s3);
	}
}

int main()
{
   const char * pstr = "abc";
   const char *pattern = "*c";
   int a = IsMatch_single(pstr,pattern);
   cout<<a<<endl;
   const char *str1 = "abc";
   const char *str2 = "abc";
   cout<<strcmp(str1,str1)<<endl;
   return 0;
}