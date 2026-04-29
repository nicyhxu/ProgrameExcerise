#include<stdio.h>
#include<algorithm>
#include<vector>
#include<string.h>
#include<iostream>
using namespace std;

string getLongestNumber(string a)
{
	int j = 0;
	int len = 0;
	unddered_map<int,int> numlen;
	for(int i = 0;i<a.length();i++)
	{
		if(a[i] <= '9' && a[i] >= '0')
		{
		    i++;
			len++;
			numlen[j] = j-i;
		}
		else
		{
			j = i;
		}			
	}
	for(int i = 0;i<numlen.size();i++)
	{
		cout<<numlen[i].first;
		cout<<numlen[i].second;
	}
	return str;
}

int main()
{
	string a = "123.4567";
	
}