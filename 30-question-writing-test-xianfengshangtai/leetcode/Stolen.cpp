#include<cstdio>
#include<algorithm>
#include<vector>
#include<string>
#include<iostream>
#include <unordered_map>  
//leetcode 474 零和1
using namespace std;


int rob(vector<int>& nums) {
    int dp[101] = {0};
	if(nums.size() == 0) return 0;
	if(nums.size() == 1) return nums[0];
	dp[nums.size()-1] = nums[nums.size()-1];
	dp[nums.size()-2] = max(nums[nums.size()-1],nums[nums.size()-2]);
	for(int i = nums.size() - 3;i >= 0;i--)
	{
		dp[i] = max(dp[i+2] + nums[i], dp[i+1]);
	}
	return dp[0];
}

int main()
{
	vector<int> nums;
	nums.push_back(1);
	nums.push_back(2);
	nums.push_back(3);
	nums.push_back(1);
	rob(nums);
    return 0;
}