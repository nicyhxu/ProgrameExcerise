#include<stdio.h>
#include<iostream>
#include<sstream>
#include <string>
#include<vector>
using namespace std;
/**
    面试题字符串反转;
*/
class Solution {
public:
    string reverseWords(string s) {
    stringstream pin(s);
    string word;
    string output;
    string exchang;
    vector<string>word_input;
    while(pin>>word && word.compare("") != 0)
    {
         word_input.push_back(word);
    }
    std:reverse(word_input.begin(),word_input.end());
    int size = word_input.size();
    int icount = 0;
    for(auto i:word_input ) {
        if(icount != size-1)
          exchang  = i + " "; 
        else
          exchang = i;
        icount++;
        output +=  exchang;
    }
     return output;
    }
};


int main()
{
	string a = "you are big world ";
    stringstream pin(a);
	string word;
	vector<string>word_input;
	while(pin>>word && word.compare("") != 0)
	{
		word_input.push_back(word);
	}
	std:reverse(word_input.begin(),word_input.end());
	//auto i=word_input.begin();
    return 0;
}