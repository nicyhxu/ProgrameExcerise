//#include"stdafx.h"
#include<iostream>
#include<algorithm>
//#include<time.h> 
//#include<Windows.h>
using namespace std;
#define N 100

struct goods {
	bool s1[N];
	int k;
	int b;
	int w; 
	int p; 
};
struct HEAP {
	KNAPNODE*p;
	int b;  //所指节点的上界;
};


bool m(goods a,goods b)
{
	return (a.p/a.w)>(b.p/b.w);
}

int max1(int a,int b)
{
	return a<b?b:a;
}
int n,W,bestP=0,cp=0,cw=0;
int X[N],cx[N];

int BackTrack(int i)
{
	if(i>n-1){
		if(bestP<cp){
			for (int k=0;k<n;k++)
				X[k]=cx[k];//存储最优路径
			bestP=cp;
		}
		return bestP;
	}
	if(cw+a[i].w<=W){       //进入左子树
		cw=cw+a[i].w;
		cp=cp+a[i].p;
		cx[a[i].sign]=1; //装入背包
		BackTrack(i+1);  
		cw=cw-a[i].w;
		cp=cp-a[i].p;    //回溯，进入右子树
	}
	cx[a[i].sign]=0; //不装入背包
	BackTrack(i+1);
	return bestP;
}

void  KnapSack3(int n,goods a[],int C,int x[])
{
	int totalW=0;
	for(int i=0;i<n;i++)
	{
		x[i]=0;
		a[i].sign=i;
	}
	std::sort(a,a+n,m);//将各物品按单位重量价值降序排列
	BackTrack(0);
	cout<<"所选择的商品如下:"<<endl;
	cout<<"序号i:\t重量w:\t价格v:\t"<<endl;
	for(int i=0;i<n;i++)
	{
		if(X[i]==1){
			cout<<i+1<<"\t";
			totalW+=b[i].w;
			cout<<b[i].w<<"\t";
			cout<<b[i].p<<endl;
		}
	}
	cout<<"放入背包的物品总重量为"<<totalW;
	cout<<"放入背包的物品总价值为"<<bestP<<endl;
}
int main()

{
	//LARGE_INTEGER begin,end,frequency; 
	//QueryPerformanceFrequency(&frequency); 
	//srand(time(0));
	cout<<"输入商品数量n和背包容量W:";
	cin>>n>>W;
	for(int i=1;i<=n;i++)
	{
		a[i].w=rand()%1000;
		a[i].p=rand()%1000;
		b[i]=a[i];
	}
	cout<<"商品的重量和价值如下："<<endl;
	for(int i=1;i<=n;i++)
	{
		cout<<a[i].w<<"\t";
		cout<<a[i].p<<endl;
	}
	//QueryPerformanceCounter(&begin); 
	KnapSack3(n,a,W,X); 
	return 0;
}
