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
	int d1 = *(int*)at;  int d2=*(int*)bt;
	vector<int>yushu1;
	vector<int>yushu2;
    if(d1 == 0) yushu1.push_back(0);
	if(d2 == 0) yushu2.push_back(0);
	while(d1) { yushu1.push_back(d1%10);d1 = d1/10;}
	while(d2) { yushu2.push_back(d2%10); d2 = d2/10;}
	reverse(yushu1.begin(),yushu1.end());
	reverse(yushu2.begin(),yushu2.end());
	int size = yushu1.size()>yushu2.size() ? yushu2.size():yushu1.size();
    for(int i=0;i<size;i++)
	{
		if(yushu1[i] > yushu2[i]) return -1;
		if(yushu1[i] == yushu2[i]) continue;
		if(yushu1[i] < yushu2[i]) return 1;
	}
	if(yushu1.size() > yushu2.size()) return -1;
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
		vector<int>bvec;
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

