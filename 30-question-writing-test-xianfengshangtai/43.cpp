#include<stdio.h>
#include<algorithm>
#include<vector>
#include<string.h>
#include<iostream>
#include <map>  
using namespace std;

#define LOCAL  1

string getLongestNumber(string a)
{
	int j = 0;
	int len = 0;
	map<int,int> numlen;
	for(int i = 0;i<a.length();i++)
	{
		if(a[i] <= '9' && a[i] >= '0')
		{
			len++;
			numlen[j] = i- j + 1;
		}
		else
		{
			j = i+1;
		}			
	}
	int Max = 0;
	int istart = 0;
	auto ptr = numlen.begin();
	while(ptr!=numlen.end())
	{
		int index = ptr->first;
		int num = ptr->second;
		if(num>=Max)
		{
			Max = num;
			istart = index;
			if(index > 0 && (a[index - 1] == '+' || a[index - 1] == '-'))
			{
				istart = index -1;
				Max = num + 1;
			}
		}
		if(index + num < a.length() && a[index + num] == '.')
		{
			int num_2 = ((++ptr)--)->second;
			if(index + num + 1 < a.length() &&
				 a[index + num + 1] <= '9' &&
				 a[index + num + 1] >= '0')
			{
				  if(num_2 + num + 1 >= Max)
				  {
					  Max = num_2 + num + 1;
				      istart = index;
					  if(index > 0 && (a[index - 1] == '+' || a[index - 1] == '-'))
					  { 
						   istart = index -1;
						   Max = Max + 1;
					  }
				  }
			}
		}
		++ptr;
	}
	return a.substr(istart,Max);
}

int main()
{
#ifdef  LOCAL
	freopen("data.in","r",stdin);
#endif
	char temp[100];
	string a;
	string b;
	while(scanf("%s",temp) != EOF)
	{
       a = temp;
	   cout<<temp<<endl;
	   b = getLongestNumber(a);
	   if(b.size() > 0)
	     cout<<b.c_str()<<endl;
	   else
		 cout<<""<<endl;
	}
}