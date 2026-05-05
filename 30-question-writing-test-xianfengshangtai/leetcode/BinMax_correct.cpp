#include<cstdio>
#include<algorithm>
#include<vector>
#include<string>
#include<iostream>
using namespace std;

// 统计 0 和 1 的个数（修复：初始化+返回正确）
int* countZeroOnes(string s) {
    int* pnew = (int*)malloc(2 * sizeof(int));
    pnew[0] = 0;  // 必须初始化！！
    pnew[1] = 0;
    
    for (int i = 0; i < s.length(); i++) {
        pnew[s[i] - '0']++;  // s[i]='0' → 0++，'1'→1++
    }
    return pnew;
}

int findMaxForm(vector<string>& strs, int m, int n) {
    int dp[101][101] = {0};  // dp[i][j]：i个0，j个1 最多多少子集
    
    for (int k = 0; k < strs.size(); k++) {
        int* a = countZeroOnes(strs[k]);
        int c0 = a[0];  // 这个字符串用几个 0
        int c1 = a[1];  // 这个字符串用几个 1
        
        // 01背包核心：必须倒序遍历！！防止重复选
        for (int i = m; i >= c0; i--) {
            for (int j = n; j >= c1; j--) {
                dp[i][j] = max(dp[i][j], dp[i - c0][j - c1] + 1);
            }
        }
        free(a);
    }
    return dp[m][n];
}

int main() {
    vector<string> str = {"10","0001","111001","1","0"};
    // 最多 5 个 0，3 个 1
    cout << "最多子集数量：" << findMaxForm(str, 5, 3) << endl; 
    return 0;
}