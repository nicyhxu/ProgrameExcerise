#include <algorithm>
#include <vector>
#include <map>
#include <iostream>
using namespace std;

int RemoveDuplicateII(vector<int>& nums)
{
    if(nums.size() <= 2) return nums.size();
	int pos = 1,flag =1; int last = nums[0];
	for(int i = 1;i<nums.size();i++)
	{
		if(nums[i] == last)
		{
			flag++;
		} 
		else
		{
		    flag = 1; 
		}
		if(flag <= 2)   //只有在什么情况下才会;
		{
			nums[pos] = nums[i];
			pos++;
		}
		last = nums[i];
	}
	return pos;
}    // O(n^2)  空间 O(1)

int RemoveDuplicateII1(vector<int>&nums)
{
    if(nums.size() <= 2) return nums.size();
	int len = 1, cnt=1; int repeat = 2;
	for(int i=1; i< nums.size();i++)
	{
      if(nums[i] != nums[len-1])
	  {
		  cnt = 1;
		  nums[len++] = nums[i];
	  }
	  else
	  {
		cnt++;
		if(cnt > repeat){
			continue;
		}
		else
		{
			nums[len++] = nums[i];
		}
	  }
	}
	return len;
}  //O(n)


int RemoveDuplicateII2(vector<int>&nums)
{
    if(nums.size() <= 2) return nums.size();
	int length = nums.size(); int step=0;
	for(int i=1;i<nums.size();i++)
	{
        step = i + 1; 
		for(;step < nums.size(); step++)
		{
			if(nums[i] != nums[step])
			{
				break;
			}
		}
		if(step -i >2)
		{
			int moveStep = step -i - 2;
			for(int j = step;j<nums.size();j++)
			{
				nums[j-moveStep] = nums[j];
			}
		    length -= moveStep;
		}
	}
	return length;
}  //O(n)

int RemoveDuplicateII3(vector<int>&nums)
{
	
}

int main()
{
   int sum = 10;
   int len = 0;
   std::vector<int>num ;//= {1,2,2,4,5,6,7,8};
   len = RemoveDuplicateII(num);
   for(int i = 0;i<num.size();i++)
   {
	   cout<<"value"<<num[i]<<endl;
   }
}