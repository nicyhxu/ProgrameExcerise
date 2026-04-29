/****
    递归的思想:需要记住卡特兰特数的性质
	C[1] = C[0] * C[1];
	C[2] = 就是一个排列组合题,排列组合题的答案，数组的通项公式
	n个数里面的按照先选取哪一个，后面的数按照同样的选择来选;
	C[n] = C[0] + C[n-1] + 但是其中有一个顺序有要求，那就是数a作为根节点，前a-1个数就是左边的子树,
	所以选择的方案一共就是who作为根节点，其他两边的树的一种组合方式;
	C[n] = C[0]*C[n-1] + C[1] *C[n-2];
	C[1] = 1;
	C[2] = C[0] *  C[1] + C[1] * C[0] = 2;
	C[3] = C[0] *  C[2] + C[1] * C[1] + C[2] * C[0] =5;
	C[4] = C[0] * C[3] + C[1] * C[2] + C[2] + C[3] * C[0]; 如果没有顺序，那么就是
***/

#include <stdio.h>
#include <vector>
#include <iostream>
#include <string.h>
using namespace std;
int main()
{
    int n  = 0;
	int i = 2;
	scanf("%d",&n);
    int a =1; 
	int b = 1;
	int temp;
	int s = 0;
	std::vector<int>sum(n+1);
	sum[1] = 1;
	sum[0] = 1;
	while(i<=n)
	{
	   for(int j=0;j<i;j++)
	   {
		   sum[i] += sum[j] * sum[i-j-1];
	   }
	   i++;
	}
	cout<<"数组求和value:"<<sum[n]<<endl;
	return 0;
}



