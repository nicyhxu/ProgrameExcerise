#include<cstdio>
#include<algorithm>
#include<vector>
#include<string>
#include<iostream>
#include <unordered_map>  
//leetcode 474 零和1
using namespace std;


int coinChange(vector<int>& coins, int amount) {
	int Max = amount + 1;
    vector<int> dp(amount + 1, Max);
	dp[0] = 0;
	for(int i = 0;i<= amount;i++)
	{
		for(int j = 0;j<coins.size();j++)
		{
			if(coins[j] <= i)
			   dp[i] = min(dp[i-coins[j]]+1,dp[i]);
		}
	}
	if(dp[amount] > amount) return -1;
    return dp[amount];
}

int main()
{
	vector<int> nums;
	nums.push_back(4);
	nums.push_back(5);
	nums.push_back(6);
	coinChange(nums,7);
    return 0;
}