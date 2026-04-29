#include<cstdio>
#include<algorithm>
#include<vector>
#include<string>
#include<iostream>
#include <unordered_map>  
//leetcode 474 零和1
using namespace std;


int minPathSum(vector<vector<int>>& grid) {
    int size = grid.size();
	if(size == 0) return 0;
	if(grid[0].size() == 0) return 0;
	int rowNum = grid.size();
	int colNum = grid[0].size();
	vector<vector<int>> dp(rowNum, vector<int>(colNum, 0));
	dp[0][0] = grid[0][0];
	for(int i = 0;i<rowNum;i++)
	{
		for(int j = 0;j<colNum; j++)
		{
			if(i>0 && j>0)
			  dp[i][j] = min(dp[i-1][j],dp[i][j-1]) + grid[i][j];
		    if(i==0)
			  dp[i][j] = dp[i][j-1] + grid[i][j];
		    if(j==0)
			  dp[i][j] = dp[i-1][j] + grid[i][j];
		}
	}
	cout<< dp[rowNum-1][colNum-1]<<endl;
}

int main()
{
	vector<int> a1 = {1,3,1};
	vector<int> a2 = {1,5,1};
	vector<int> a3 = {4,2,1}
	vector<vector<int>>grid = {a1,a2,a3};
	minPathSum(grid);
}