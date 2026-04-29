#include <algorithm>
#include <vector>
#include <map>
#include <iostream>
using namespace std;

/*
*  类似与数组去重复的操作;
*/
int MoveZero(vector<int>& nums)
{
	int klen = 0;
	for( int i = 0;i<nums.size() - klen;)
	{
		if(nums[i] == 0)
		{
		   for(int j= i; j< nums.size()-klen-1;j++)
		   {
		       nums[j] = nums[j+1];
			   nums[j+1] = 0;
		   }
	       klen++;
		}
		else
		{
		   i++;
		}
	}
	return nums.size() - klen;
}    // O(n^2)  空间 O(1)

/*
*  获取股票的最大值,可以交易多次;
*/

int main()
{
   int sum = 10;
   int klen = 0;
   std::vector<int>num = {0,0,1,2,2,4,0,0,0,5,6,7,8};
   klen = MoveZero(num);
   for(int i = 0;i< klen;i++)
   {
	   cout<<"value"<<num[i]<<endl;
   }
}