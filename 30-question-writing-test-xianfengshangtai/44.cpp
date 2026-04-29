#include<stdio.h>
#include<algorithm>
#include<vector>
#include<string.h>
#include<iostream>
#include <map>  
using namespace std;

void privot_sort_fast(vector<int>&orign,int begin,int end)
{
	if(begin == end) return;
	int val = orign[end];
	int privot = end;
	int first = begin;
	int last = end;
	while(first<last)
	{ 
		while(vector[first] < orign[privot]) first++;
		while(vector[last] > orign[privot]) last--;
		if(first<last) 
		{
		   swap(&orign[first],&orign[last],sizeof(int));
		}
	}
	orign[first] = val;
	privot_sort_fast(orign,begin,first-1);
	privot_sort_fast(orign,first+1,end);
}

void sortfast(vector<int>&orign)
{
	privot_sort_fast(orign,0,orign.size()-1);
}

int main()
{
#ifdef  LOCAL
	freopen("data.in","r",stdin);
#endif
    char pstr[100];
	char * p = NULL;
	vector<int>avec;
    while(scanf("%s",pstr)!=EOF)
	{
		p = strtok(pstr,",");
		while(p)
		{
			avec.push_back(p[0]-'0');
			p = strtok(NULL,",");
		}
		sortfast(avec);
	}
}