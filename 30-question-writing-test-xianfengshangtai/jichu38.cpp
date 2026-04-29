#include<cstdio>
#include<algorithm>
#include<vector>
#include<string>
#include<iostream>
using namespace std;

int SortByShell(vector<int> & shell)
{
	int isize = shell.size();
	int inc = 1;
	int value = 0;
	do{
		inc *=3;  //inc肯定是3的倍数;
		inc++;
	}while(inc<isize);
    do{
		inc/=3;
		for(int i= inc;i<isize;i++)
		{
			value = shell[i];
			int j = i;
			if(shell[j-inc] > value)
			{
				shell[j] = shell[j-inc];
				j -= inc;
				if(j<0)
					break;
			}
			shell[j] = value;
		}
	}while(inc>1);	
	return 0;
}

int main()
{
   vector<int> vsort ;//= {4,6,7,5,8,9,3}
   SortByShell(vsort);
   for(int i = 0;i<vsort.size();i++)
   {
	   cout<<vsort[i]<<","<<endl;
   }
}