#include<cstdio>
#include<algorithm>
#include<vector>
#include<string>
#include<iostream>
using namespace std;

// LeetCode 322 零钱兑换（完全背包）
int coinChange(vector<int>& coins, int amount) {
    int Max = amount + 1;
    vector<int> dp(amount + 1, Max);
    dp[0] = 0;  // 0 元需要 0 枚硬币
    
    // 完全背包正确顺序：先物品，再背包容量
    for(int j = 0; j < coins.size(); j++) {  // 遍历硬币
        for(int i = coins[j]; i <= amount; i++) {  // 遍历金额
            dp[i] = min(dp[i - coins[j]] + 1, dp[i]);
        }
    }
    
    // 无法凑成返回 -1
    return dp[amount] > amount ? -1 : dp[amount];
}

int main() {
    vector<int> nums;
    nums.push_back(4);
    nums.push_back(5);
    nums.push_back(6);
    
    // 输出结果
    cout << coinChange(nums, 7) << endl;
    return 0;
}