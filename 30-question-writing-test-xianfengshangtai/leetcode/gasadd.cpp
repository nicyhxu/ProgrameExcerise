#include <algorithm>
#include <vector>
#include <map>
#include <queue>
#include <string>
#include <set>
#include <iostream>
using namespace std;
#define LOCAL 1
#define FileName "gasadd.in"

int canCompleteCircuit(vector<int>& gas, vector<int>& cost) 
{
	 int cursum = 0;
	 int totalsum = 0;
	 int start = 0;
	 for(int i=0;i<gas.size();i++)
	 {
		 totalsum += gas[i] - cost[i];
		 cursum += gas[i] - cost[i];
		 if(cursum < 0)
		 {
			 start = i + 1;
			 cursum = 0;
		 }
	 }
	 if(totalsum < 0 ) return -1;
	 return start;
}

int main()
{
#ifdef  LOCAL
	freopen(FileName,"r",stdin);
#endif
    char pstr[100];
	char * p = NULL;
	int count = 0;
	vector<int>avec;
	vector<int>bvec;
    while(scanf("%s",pstr)!=EOF)
	{
		p = strtok(pstr,",");
		if(count%2 == 0)
		{
			while(p)
			{
				avec.push_back(atoi(p-'0'));
				p = strtok(NULL,",");
			}
		}
		else
		{
			while(p)
			{
				bvec.push_back(atoi(p-'0'));
				p = strtok(NULL,",");
			}
			int ret = canCompleteCircuit(avec,bvec);
			cout<<ret<<endl;
			avec.clear();
			bvec.clear();
		}
	}
}
