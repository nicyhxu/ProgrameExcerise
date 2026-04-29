#include <stdio.h>
#include <algorithm>
#include "time.h"
#include <stdlib.h>

using namespace std;

static int h[11] = {0};
static int c[11] = {0};
static int a[11] = {0};
int n = 10;

int lowbit(int x)
{
    return x&(x^(x-1));
}

int lowbit2(int x)
{
	return x&(-x);
}

//更新c数组的方法;
void plus(int pos,int num)
{
	a[pos] += num;
	while(pos < n)
	{
		c[pos] += num;
		pos += lowbit(pos);
	}
}

void build(int n)
{
	for(int i=1;i<=n;i++)
	{
		c[i] += a[i];
		for(int j=1;j<lowbit(i);j<<=1)
		{
		    c[i] += c[i-j];
			printf("j %d value add %d value %d \n",i, c[i-j],c[i]);
		}
	}
}

//数组求和的做法;
int sum(int end)
{
	int sum = 0;
	while(end > 0)
	{
		sum += c[end];
		end -= lowbit(end);
	}
	return sum;
}

//nlogn的数组最大值维护
void update(int i,int val)
{
	while(i<n)
	{
		h[i] = max(h[i],val);
		i += lowbit(i);
	}
}

void update( int x)
{
	int lx,i;
	while(x <= n)
    {
		h[x] = a[x];
		lx = lowbit(x);
		for(i=1;i<lx;i<<=1)
		{
			printf("update i %d value %d \n",i, x);
			h[x] = max(h[x],h[x-i]);
		}
		x +=lowbit(x);
	}
}

/*
**  如何求解x,y区间的最大值;区间
**  h[y] 表示的是[y,y-lowbit(y)+1]的最大值;
**  所以可以这样求解: y-lowbit(y)>x,则 query(x,y) = max(h[y],query(x,y-lowbit(y));
**  如果y-lowbit(y)<=x，那么query(x,y) = max(a[y],query(x,y-1));	
*/
int query(int x,int y)
{
	int ans = 0;
	while(y>=x)
	{
		ans = max(a[y],ans);
		y--;
		for(;y-lowbit(y)>=x;y-=lowbit(y))
			ans = max(h[y],ans);
	}
	return ans;
}

int main()
{
    for(int i = 1 ;i<=10;i++)
	{
		int j = rand()%10;
		a[i] = j;
	}
	build(10);
	for(int i=1;i<10;i++)
	{
		printf("i %d value %d \n",i, a[i]);
		printf("i %d value %d \n",i, c[i]);
	}
	plus(1,3);
	printf("after plus\n");
	printf("after plus\n");
	for(int i=1;i<10;i++)
	{
		printf("i %d value %d \n",i, a[i]);
		printf("i %d value %d \n",i, c[i]);
	}
	printf("3&(3^2) %d \n",lowbit(1));
	printf("sum end pos 4 %d \n",sum(5));
	//测试最大值的维护;
	printf("after update max\n");
	for(int i=1;i<10;i++)
	{
		update(i);
	}
	for(int i=1;i<10;i++)
	{
		printf("i %d value %d \n",i, h[i]);
	}
}
/*c99
涉及到整数转换为指针
一般有两种情况会用到：
句柄是一种指向指针的,但是句柄里面的值是不会变化的;
线程函数参数;
runing error:
jichu:10 is ok when a is a vaild address;
这里编译时没有问题的,整数转换成指针;这里的整数其实是地址;
*/