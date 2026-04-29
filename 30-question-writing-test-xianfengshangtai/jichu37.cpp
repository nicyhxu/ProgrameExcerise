#include<cstdio>
#include<algorithm>
#include<vector>
#include<string>
#include<iostream>
using namespace std;

std::vector<string>pstrVec;

void Permutation(char*pStr,char*pBegin)
{
	if(*pBegin=='\0')
	{
	    string tmp(pStr);
		pstrVec.push_back(tmp);
	}
	else
	{
       for(char*pCh=pBegin;*pCh!='\0';++pCh)
	   {
		   char temp= *pCh;
		   *pCh=*pBegin;
		   *pBegin=temp;
		   Permutation(pStr,pBegin+1);
		   temp=*pCh;
		   *pCh=*pBegin;
		   *pBegin=temp;
	   }
	}
}
bool isHuiwen(string s,int size) {
if(size == 1) {
	return true;
}
for(int i=0;i<=(size-1)/2;++i) {
	if(s[i] != s[size-1-i]) {
		return false;
	 }
}
return true;
}

int getLongestPalindrome(string str,int n){
	vector<vector<int> > dp(n,vector<int>(n));
	int maxvalue=0;
	for(int i=n-1;i>=0;i--){
		dp[i][i]=1;
		for(int j=i+1;j<n;j++){
			if(str[i]==str[j] && isHuiwen(str.substr(i,j-i+1),j-i+1)){	
				dp[i][j]=dp[i+1][j-1]+2;  
			}else {
				dp[i][j]=max(dp[i+1][j],dp[i][j-1]);  
			}
		}
	}
	return dp[0][n-1];
}
	
int main()
{
	char pch;
	char quhar[1024];
	int max=0;
	int n = 0;
    while((pch=getchar()) != '\n')
	{
		if(pch >='1' && pch<='9')
		{
			quhar[n++]=pch;
		}
   }
   quhar[n]='\0';
   Permutation(quhar,quhar);
   for(int i=0;i<pstrVec.size();i++)
   {
	   int imax = getLongestPalindrome(pstrVec[i],pstrVec[i].length());
	   if(imax > max)
		   max = imax;
   }
   cout<<max<<endl;
}