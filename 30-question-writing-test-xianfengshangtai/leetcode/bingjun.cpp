#include <algorithm>
#include <vector>
#include <map>
#include <queue>
#include <string>
#include <set>
#include <iostream>
using namespace std;
#define LOCAL 1
#define FileName "big.in"
int a[20] = {0};
int cstring_cmp(const void* at,const void* bt)
{
	return 0;
}

void printvec(vector<int>&temp)
{
	vector<int>firstthree;
	for(int i=0;i<temp.size();i++) 
	a[i] = temp[i];
	qsort(a,temp.size(),sizeof(int),cstring_cmp);
	for(int i =0 ;i<temp.size();i++) cout<<a[i];
	cout<<endl;
	return;
}

int main()
{
#ifdef  LOCAL
	freopen(FileName,"r",stdin);
#endif
    string pstr;
	char * p = NULL;
	int count = 0;
    while(getline(cin,pstr))
	{
		vector<char>bvec;
		p = strtok((char*)pstr.c_str()," ");
		while(p)
		{
		    bvec.push_back(atoi(p));
			p = strtok(NULL," ");
		}
		//sort(bvec.begin(),bvec.end());
		printvec(bvec);
	}
}

