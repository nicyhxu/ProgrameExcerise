#include<stdio.h>
#include<algorithm>
#include<vector>
#include<string.h>
#include<iostream>
using namespace std;

std::vector<int> min_vec;
std::vector<int> max_vec;
int main()
{
	char s[105];
	char * p = NULL;
	int min_vec_sort[1000];
	 char * ptemp = NULL;
	 char * ptemp2 = NULL;
    std::vector<string>pstr;
	scanf("%s",&s);
    p = strtok(s,",");
    while(p)
	{
		string a(p);
		pstr.push_back(a);
		p = strtok(NULL,",");
	}
	for(int i=0;i<pstr.size();i++)
	{
        int index = pstr[i].find("-");
		string tem = pstr[i].substr(0,index);
		string tem2 = pstr[i].substr(index+1,pstr[i].length()-1);
		int min = atoi(tem.c_str());
		int max = atoi(tem2.c_str());
		min_vec.push_back(min);
		max_vec.push_back(max);	
	}
	for(int i=0;i<min_vec.size();i++)
	{
		for(int j=i;j< min_vec.size();j++)
		{
			if((min_vec[j] >= min_vec[i]) && (min_vec[j] <= max_vec[i]) ){
				int max = max_vec[j] > max_vec[i] ? max_vec[j] : max_vec[i];
				min_vec[i] = min_vec[i];
				min_vec[j] = min_vec[i];
				max_vec[j] = max;
				max_vec[i] = max;
			    min_vec_sort[i] = min_vec[i];
			}
			if((min_vec[i] >= min_vec[j]) && (min_vec[i] <= max_vec[j]))
			{
				int max = max_vec[j] > max_vec[i] ? max_vec[j] : max_vec[i];
				min_vec[i] = min_vec[j];
				min_vec[j] = min_vec[j];
				max_vec[j] = max;
				max_vec[i] = max;
			    min_vec_sort[i] = min_vec[i];
			}
			else
			{
				min_vec_sort[i] = min_vec[i];
				min_vec_sort[j] = min_vec[j];
				cout<<min_vec_sort[i]<<min_vec_sort[j]<<endl;
			}
		}
	}

	sort(min_vec_sort,min_vec_sort+ min_vec.size());
	int c_num = unique(min_vec_sort,min_vec_sort+ min_vec.size()) - min_vec_sort;
    for(int i= 0; i< c_num; i++)
	{
		for(int j= min_vec.size()-1;j>=0;j--)
		{
			if(min_vec[j] == min_vec_sort[i] && i < c_num - 1)
			{
				cout<<min_vec[j]<<"-"<<max_vec[j]<<",";
				break;
			}
			if(min_vec[j] == min_vec_sort[i] && i == c_num - 1 )
			{
				cout<<min_vec[j]<<"-"<<max_vec[j]<<endl;
				break;
			}
		}
	}
}