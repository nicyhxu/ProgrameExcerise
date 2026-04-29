#include <algorithm>
#include <vector>
#include <map>
#include <queue>
#include <string>
#include <set>
#include <iostream>
#include<numeric>

using namespace std;

/* 此模板函数就是参考iota的实现 */
template<class ForwardIterator, class T>
void iotc(ForwardIterator  first, ForwardIterator last, T value)
{
    while(first != last) {
        *first++ = value;
        ++value;
    }
}

#define LOCAL 1
#define FileName "IslandArea.in"
/*C++ 11新增了许多新特性，如下为生成递增值的函数*/
class UniounFn {
public:
     UniounFn(int nodeNum):m_parent(nodeNum,0),m_rank(nodeNum,0)
	 {
		iota(m_parent.begin(),m_parent.end(),0);
	 }
	 /* 如何对联通子集合进行合并策略 */
	 void mergeUnion(int x, int y)
	 {
		 //肯定是寻找对应的联通数据,每个都是自己的父亲节点开始
		 int px = getparent(x);
		 int py = getparent(y);
		 if(px == py) return;
		 if(px > py) {
			 m_parent[x] = py;
		     m_rank[py] += m_rank[x];
		 }else
		 {
			 m_parent[y] = px;
		     m_rank[px] += m_rank[py];
		 }
	 }
	 int getParent(int x)
	 {
		 if(m_parent[x] = x) return x;
		 while(m_parent[x] != x) x=m_parent[x];
		 return m_parent[x];
	 }
public:
	std::vector<int>m_parent;
	vector<int>m_rank;
};

int main()
{
#ifdef  LOCAL
	freopen(FileName,"r",stdin);
#endif
    string pstr;
	char * p = NULL;
	int count = 0;
	int row,col;
	while(scanf("%d,%d",&row,&col) != EOF)
	{
		UniounFn(rpw*col);  //总共多少个节点是有要求的;
		vector<vector<char>>charMaxtrix;
		for(int input = 0; input < row ; input++)
		{
		   while(getline(cin,pstr))
	       {
				vector<char>bvec;
				p = strtok((char*)pstr.c_str(),",");
				while(p)
				{
					bvec.push_back(p);
					p = strtok(NULL," ");
				}
				charMaxtrix.push_back(bvec);
	       }
		}
		//如何进行合并，是需要判断左右还有上下的;
		for(int idx = 0;idx < row - 1;idx ++) {
			for(int idj = 0; idj < col - 1 ; idj++) {
				if(charMaxtrix[idx][idj] == charMaxtrix[idx][idj+1]) {
					UniounFn.mergeUnion(idx * col + idj,idx * col +idj +1);
				}
				if(charMaxtrix[idx][idj] == charMatrix[idx+1][idj]) {
					UniounFn.mergeUnion(idx * col + idj,idx * col + col);
				}
			}
		}
		//得到合并后的数据;
	}
}

