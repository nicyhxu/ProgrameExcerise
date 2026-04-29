#include <algorithm>
#include <vector>
#include <map>
#include <iostream>
using namespace std;

/*
*  获取股票的最大值,只能交易一次;
*/
int StockProfit(vector<int>& nums)
{
	int sum = 0;
	int min = 0;
	for( int i = 0;i<nums.size()-1;i++)
	{
		if(nums[i] < min)
		{
		   min = nums[i];
		}
		else
		{
			if(nums[i] - min > sum)
			  sum = nums[i] - min;
		}
	}
	return sum;
} //滑动窗口;

/*
*  获取股票的最大值,可以交易多次;
*/
int StockProfit1(vector<int>& nums)
{
   int sum = 0;
   for(int i = 0;i<nums.size();i++)
   {
	   if(nums[i+1] > nums[i])
	   {
		   sum += nums[i+1] - nums[i];
	   }
	   else
	   {
		   continue;
	   }
   }
   return sum;
}

int StockProfit2(vector<int>& nums)
{
	int sum = 0;
	for(int i = 0;i<nums.size();i++)
	{
		for(int j = i;j<nums.size();j++)
		{
			if(j+2 == nums.size() && nums[j+1] >= nums[j])
			{
				sum += nums[j+1] - nums[i];
				i = j+1;
				break;
			}
			if(nums[j] >= nums[j+1])
			{
				sum += nums[j] - nums[i];
				i = j;
				break;
			}
		}
	}
	return sum;
} //滑动窗口


int main()
{
   int sum = 10;
   int profit = 0;
   std::vector<int>num ;//= {1,2,2,4,5,6,7,8};
   profit = StockProfit(num);
   cout<<"MaxProfit"<<profit<<endl;
}