#include <algorithm>
#include <vector>
#include <map>
#include <queue>
#include <string>
#include <set>
#include <iostream>
using namespace std;
struct ListNode 
{
	int val;
	struct ListNode *next;
	ListNode(int x) :
	val(x), next(NULL) 
	{
	}
};


int main()
{
     ListNode *head = new ListNode(1);
	 ListNode *ptmp = head;
	 for(int i =0;i<5;i++)
	 {
		 ListNode *pphead = new ListNode(i);
		 ptmp->next = pphead;
		 ptmp = pphead;
	 }
	 return 0;
}

