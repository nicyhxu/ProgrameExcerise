/*242. 有效的字母异位词

给定两个字符串 s 和 t ，编写一个函数来判断 t 是否是 s 的字母异位词。

示例 1:

输入: s = "anagram", t = "nagaram"
输出: true

示例 2:

输入: s = "rat", t = "car"
输出: false

说明:
你可以假设字符串只包含小写字母。
*/

#include<stdio.h>
#include<hash_map>
#include<iostream>
using namespace std;

int main()
{
   string c1 = "anagram";
   string c2 = "nagaram";
   hash_map<char,int>count1;
    hash_map<char,int>count2;
   for(int i=0;i<c1.size();i++)
   {
	   count1[c1[i]]++;
   }
   for(int i=0;i<c2.size();i++)
   {
	   count2[c2[i]]++;
   }
   for(char i= 'a'; i< 'z';i++)
	   if(count1[i] != count2[c2[i]])
		   return false
   return true;
}
