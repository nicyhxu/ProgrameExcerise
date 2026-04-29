#include <algorithm>
#include <vector>
#include <map>
#include <iostream>
using namespace std;

int RemoveDuplicate(vector<int>& nums)
{
    int len = nums.size();
	for(int i=0;i<len;)  
	{
		if(nums[i] == nums[i-1]) {
			for(int j=i;j<len;j++)
			   nums[j-1] = nums[j];
		    len--;
		}
		else
		{
		    i++; 
		}
	}
	return len;
}    // O(n^2)  空间 O(1)

int RemoveDuplicate1(vector<int>&nums)
{
	int count = 0;
	if(nums.size() == 0) return 0;
	for(int i = 1;i<nums.size();i++)
	{
	   if(nums[i-1] != nums[i])
	   {
		   nums[count++] = nums[i];
	   }
	}
	return count;
}  //O(n)

int RemoveDuplicate2(vector<int>& nums)
{
	if(nums.size() == 0) return 0;
	int i = 0;
	for(int j=1;j<nums.size();j++)
	{
		if(nums[i] != nums[j])
		{
			i++;
			nums[i] = nums[j];
		}
			
	}
	return i+1;
}

int main()
{
   int sum = 10;
   int len = 0;
   std::vector<int>num ;//= {1,2,2,4,5,6,7,8};
   len = RemoveDuplicate1(num);
   for(int i = 0;i<num.size();i++)
   {
	   cout<<"value"<<num[i]<<endl;
   }
}