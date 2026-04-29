#include <algorithm>
#include <vector>
#include <map>
#include <iostream>
using namespace std;

#define Max 10000

int main()
{
   int bridge_length;
   int MinStep;
   int MaxStep;
   int StoneNum;
   int stone_dp[10000];
   int an[10000] = {0};
   cin>>bridge_length;
   cin>>MinStep>>MaxStep>>StoneNum;
   for(int i=0;i<StoneNum;i++)
   {
	   int temp_index;
	   cin>>temp_index;
	   an[temp_index] = 1;
   }
   stone_dp[bridge_length-MaxStep] = an[bridge_length-MaxStep];
   for(int i=0;i<MaxStep;i++)
   {
      stone_dp[bridge_length-i] = an[bridge_length-i];
	  cout<<"xunhuan"<<stone_dp[bridge_length-i];
   }
   for(int i=bridge_length - MaxStep -1;i>=0;i--)
   {
	   int min_dp = stone_dp[i + MinStep];
	   for(int j = MinStep; j<= MaxStep; j++)
	   {
		   min_dp = min(min_dp,stone_dp[i + j]);
	   }
	   stone_dp[i] = min_dp + an[i];
   }
   cout<<stone_dp[0]<<endl;
}