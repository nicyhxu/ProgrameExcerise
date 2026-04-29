#include<stdio.h>
#include<sstream>
#include<iostream>
#include<string>
using namespace std;
class Solution {
public:
#define MAXN 50000
    static int cstring_cmp(const void*a,const void*b)
    {
	    const char **ia = (const char **)a;
	    const char **ib = (const char **)b;
	    return strcmp(*ia,*ib);
    }
    static int comlen(char* p, char* q)
    {
	    int i = 0;
	    while (*q && (*p++ == *q++))
	    {
		    i++;
	    }
	    return i;
    }
    string longestPalindrome(string s) {
	char c1[MAXN];
    string output;
	int an = 0;
	int bn = 0;
	int ref = -1;
	int maxlen = 0;
	int maxi = 0;
	int maxib = 0;
    int len = 0;
	int length = s.size();
	for(int i=0;i<s.size();i++)
		c1[i] = s[i];
	c1[length] = '\0';
	char *a[MAXN];
	for(int i=0;i<s.size();i++)
	{
		a[i] = &c1[i];
	}
	qsort(a, length, sizeof(char*), cstring_cmp);
	char c2[MAXN];
	for(int i=0;i<s.size();i++)
	{
		c2[i] = c1[length-i-1];
	}
	c2[length] = '\0';
	char *b[MAXN];
	for(int i=0;i<s.size();i++)
	{
		b[i] = &c2[i];
    }
	qsort(b, length, sizeof(char*), cstring_cmp);
	while (an < length  && bn < length )
	{
		len = comlen(a[an], b[bn]);
		if ((maxlen < len) && ((length - strlen(a[an])) + len) == (strlen(b[bn])))
		{
			maxlen = len;
			maxi = an;
			maxib = bn;
		}
		ref = strcmp(a[an], b[bn]);
		if (ref == 1)
		{
			bn++;
		}
		else
		{
			an++;
		}
	}
	char ch_tmp;
	for (int i = 0; i < maxlen; i++)
	{
		ch_tmp = *(a[maxi] + i);
        output +=ch_tmp;
	}
	cout<<output<<endl;
    return output;
    }
};
int main()
{
	string s1= "cbbd";
	string s2 = "babad";
	Solution*psolution = new Solution();
	psolution->longestPalindrome(s1);
	psolution->longestPalindrome(s2);
	delete psolution;
	return 0;
}