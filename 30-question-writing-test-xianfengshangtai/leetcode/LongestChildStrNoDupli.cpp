#include<cstdio>
#include<algorithm>
#include<vector>
#include<string>
#include<iostream>
#include <unordered_map>  
using namespace std;


int lengthOfLongestSubstring(string s) {
   unordered_map<char,int>char_map;
   unordered_map<char,int>char_pos;
   int iend = 0;
   int istart = 0;
   int maxlen = 0;
   int maxi = 0; 
   int maxj = 0;
   for(int j = iend;j< s.length();j++)
   {
		char temp = s[j];
		cout<<"char"<<temp<<"num"<<char_map[temp]<<endl;
		if( ++char_map[temp] > 1 && char_pos[temp] >= istart)
		{
			iend = j + 1;
			char_map[temp] = 1;
			if(j - istart >= maxlen)
			{
				maxi = istart;
				maxj = j -1;
				maxlen = j - istart;
				cout<<"maxlen"<<maxlen<<endl;
			}
			istart = char_pos[temp] + 1;
			cout<<istart<<endl;
		}
		char_pos[temp] = j;
		if(j == s.length() -1 && j - istart + 1>= maxlen)
		{
			maxi = istart;
			maxj = j;
			maxlen = j - istart + 1;
			cout<<"maxlen"<<maxlen<<endl;		
		}
   }
   cout<<maxlen<<endl;
   return  0;
}

int main()
{
	string str;
	cin>>str;
    lengthOfLongestSubstring(str);
	return 0;
}