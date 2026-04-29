#include <algorithm>
#include <vector>
#include <map>
#include <queue>
#include <string>
#include <set>
#include <iostream>
using namespace std;
#define LOCAL 1
#define FileName "IslandArea.in"


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
		p = strtok((char*)pstr.c_str(),"]");
		while(p)
		{
		    bvec.push_back(atoi(p));
			p = strtok(NULL," ");
		}
		//sort(bvec.begin(),bvec.end());
		printvec(bvec);
	}
}

