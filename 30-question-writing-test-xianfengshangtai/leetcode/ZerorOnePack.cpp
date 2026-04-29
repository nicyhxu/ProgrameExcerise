#include<cstdio>
#include<algorithm>
#include<vector>
#include<string>
#include<iostream>
#include <unordered_map>  
//leetcode 474 零和1
using namespace std;
int w[105],val[105];
int dp[105][1005];

/*
 0 1背包，没有优化空间的方法;t从大到小或者小到大
*/
int main()
{
    int t,m,res = -1;
	cin >> t >> m;
    for(int i=1; i<=m; i++)
        cin >> w[i] >> val[i];
    
    for(int i=1; i<=m; i++) //物品 
        for(int j=0; j<=t; j++) //容量 
    {
        if(j >= w[i])
           dp[i][j] = max(dp[i-1][j-w[i]]+val[i], dp[i-1][j]);
        else      //只是为了好理解
           dp[i][j] = dp[i-1][j];           
    }
    cout << dp[m][t] << endl;
    return 0;
}