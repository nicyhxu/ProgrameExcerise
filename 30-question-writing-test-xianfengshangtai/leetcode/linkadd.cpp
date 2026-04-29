#include <algorithm>
#include <vector>
#include <map>
#include <iostream>
using namespace std;

#define LOCAL 1

struct ListNode {
     int val;
     ListNode *next;
     ListNode() : val(0), next(nullptr) {}
     ListNode(int x) : val(x), next(nullptr) {}
     ListNode(int x, ListNode *next) : val(x), next(next) {}
};

ListNode* addTwoNumbers(ListNode* l1, ListNode* l2) {
    int len1 = 1;
	int len2 = 1;
	ListNode * p1 = l1;
	ListNode * p2 = l2;
    while(p1->next)
	{
		len1++;
		p1 = p1->next;
	}
	while(p2)
	{
		len2++;
		p2 = p2->next;
	}
	int length = 0;
	if(len1>len2)
	{
		for(int i = 0; i< len1 - len2; i++){
			ListNode * tmp = new ListNode(0);
			p2->next = tmp;
		}	
		length = len1;
	}
	else
	{
		for(int i =0; i<len2-len1;i++)
		{
			ListNode *tmp = new ListNode(0);
			p1->next = tmp;
		}
		length = len2;
	}
	ListNode * pheader = new ListNode(-1);
    for(int i = 0 ;i< length -1;i++)
	{
		ListNode * ptemp = new ListNode(-1);
		pheader->next = ptemp;
	}
	int curse = 0;
	ListNode *q1 = l1;
	ListNode *q2 = l2;
	ListNode * q3 = pheader;
	while(q3)
	{
		int val = q1->val + q2->val + curse;
		if(val >= 10) 
		{
			curse = 1;
			val = val -10;
		}
		q3->val = val;
		q1 = q1->next;
		q2 = q2->next;
		q3 = q3->next;
	}
    return pheader;
}

void tranverse(ListNode * p)
{
	while(p)
	{
	   cout<<p->val<<","<<endl;
	   p = p->next;
	}
}
#define Filename "LinkTree.in"	

int main()
{
#ifdef  LOCAL
	freopen(Filename,"r",stdin);
#endif
	char temp[100];
	string a;
	string b;
	char * p = NULL;
	ListNode * pHeader1 = new ListNode(-1);
	ListNode * pHeader2 = new ListNode(-1);
	ListNode *ap1 = pHeader1;
	ListNode * ap2 = pHeader2;
	ListNode *pheader = NULL;
	ListNode * ap = NULL;
	int count = 0;
	while(scanf("%s",temp) != EOF)
	{
        p = strtok(temp,",");
		if(count%2 == 0) 
		{
			pheader = pHeader1;
			ap = ap1;
		}
		else
		{
			pheader = pHeader2;
			ap = ap2;
			tranverse(pHeader1);
		}
		pheader->val = p[0] - '0';
        while(p)
	    {
		  string a(p);
		  p = strtok(NULL,","); 
		  ListNode * tmp = new ListNode(-1);
		  tmp->val = p[0] - '0';
		  ap->next = tmp;
		  ap = ap->next;
	   }
	   count++;
	}
}

