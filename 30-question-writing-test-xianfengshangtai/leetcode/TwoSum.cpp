#include <algorithm>
#include <vector>
#include <map>
#include <iostream>
using namespace std;
vector<int> twoSum(vector<int>& nums, int target)
{
	std::vector<int>res;
	std::map<int,int>ptemp_hash;
	for(int i = 0;i<nums.size();i++)
	{
		if(ptemp_hash.count(target-nums[i]) > 0)
			return {i, ptemp_hash[target - nums[i]]};
		ptemp_hash[nums[i]] = i;
	}
	return {};
}

int main()
{
   int sum = 10;
   std::vector<int>num = {1,2,2,4,5,6,7,8};
   std::vector<int>res = twoSum(num,sum);
   for(int i = 0;i<res.size();i++)
   {
	   cout<<"value"<<res[i]<<endl;
   }
}