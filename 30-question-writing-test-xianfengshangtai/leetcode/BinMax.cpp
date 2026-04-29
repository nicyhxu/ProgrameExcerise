#include<cstdio>
#include<algorithm>
#include<vector>
#include<string>
#include<iostream>
#include <unordered_map>  
//leetcode 474 零和1
using namespace std;

int* countZeroOnes(string s)
{
	int * pnew  = (int *)malloc(2*sizeof(int));
	for(int i = 0;i<s.length();i++)
	{
		pnew[s[i] - '0']++;
	}
	return pnew;
}

int findMaxForm(vector<string>& strs, int m, int n){
   int dp[101][101] = {{0}};
   int countZero;
   int countOne;
   for(int k = 0 ;k<strs.size(); k++)
   {
	    int * a =  countZeroOnes(strs[k]);
		for(int i = 0; i< m;i++)
		{
		   for(int j = 0;j< n;j++)
		   {
			   dp[i][j] = max(dp[i-a[1]][j-a[0]] + 1,dp[i][j]);
		   }
		}
		free(a);
   }
   return  dp[m][n];
}

int main()
{
	string a = "10";
	string b = "0001";
	string c = "111001";
	string d = "1";
	string e = "0";
	vector<string> str;
	str.push_back(a);
	str.push_back(b);
	str.push_back(c);
	str.push_back(d);
	str.push_back(e);
	findMaxForm(str,5,3);
    return 0;
}