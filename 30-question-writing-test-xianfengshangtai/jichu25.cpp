#include <iostream>
#include <cstdio>
#include <sstream>
using namespace std;
int main()
{
  string zimu;
  while(getline(cin,zimu))
  {
	int num[26] ={0};
	int min = 10000;
	int max = 0;
	string result;
	for(int i=0;i<zimu.length();i++)
	{
		int temp = zimu[i]-'a';
		num[temp]++;
	}
	for(int i=0;i<26;i++)
	{
		if(num[i]<min && num[i]>0) min=num[i];
	}
	for(int i=0;i<zimu.length();i++)
	{
		int temp = zimu[i]-'a';
		if(num[temp] > min)
		{
		  result += zimu[i];
		}
	}
	cout<<result<<endl;
  }
  return 1;
}