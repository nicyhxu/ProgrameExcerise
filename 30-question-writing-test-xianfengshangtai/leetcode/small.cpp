#include "string.h"
#include <algorithm>
#include <vector>
#include <map>
#include <iostream>
using namespace std;

#define Filename "small.in"	

int main()
{
#ifdef  LOCAL
	freopen(Filename,"r",stdin);
#endif
    vector<int> a;
	int b = 0;
	int count = 0;
	while(scanf("%s",temp) != EOF)
	{
		p = strtok(temp,",");
		if(count%2 == 0) 
		{
            a.push_back(atoi(p[i]));
			p = strtok(NULL,",");
		}
		else
		{
			b = atoi(p);
			sort(a.begin(),a.end());
			int sum = 0;
			for(int i=0;i<a.size();i++)
			{
				if(sum + a[i] > b) break;
				sum +=a[i];
			}
			cout<<i<<endl;
		}
       a.clear();
	   count++;
	}
}

