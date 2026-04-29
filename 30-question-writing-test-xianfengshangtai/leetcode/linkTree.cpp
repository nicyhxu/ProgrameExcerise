#include <algorithm>
#include <vector>
#include <map>
#include <queue>
#include <iostream>
using namespace std;

#define LOCAL 1

class Node {
public:
    int val;
    Node* left;
    Node* right;
    Node* next;

    Node() : val(0), left(NULL), right(NULL), next(NULL) {}

    Node(int _val) : val(_val), left(NULL), right(NULL), next(NULL) {}

    Node(int _val, Node* _left, Node* _right, Node* _next)
        : val(_val), left(_left), right(_right), next(_next) {}
};

Node* connect(Node* root) {
    queue<Node*> pNode;
	Node *ptr = root;
	pNode.push(ptr);
	while(!pNode.empty())
	{
		int size = pNode.size();
		while(size--)
		{
			ptr = pNode.front(); pNode.pop();
			ptr->next = size ? pNode.front():NULL;
			if(ptr->left) pNode.push(ptr->left);
			if(ptr->right) pNode.push(ptr->right);   //left和right都添加;
		}
	}
	return root;
}

void DFS(Node*root,vector<int> &prev ,vector<int>in)
{
	if(root == NULL)
		return;
	vector<int>leftprevchild;
	vector<int>rightprevchild;
	vector<int>leftinchild;
	vector<int>rightinchild;
	int roval = root->val;
	for(int i = 0;i<in.size(); i++)
	{
		int findin = 0;
		if(roval == in[i])
		{
			findin =1;
			Node * pNode = new Node(prev[1]);
			root->left = pNode;
			if(i< in.size()-1)
			{
				Node *pNode2 = new Node(prev[i+1]);  //为什么是i+1,证明有这么多个数据;如果i+1超出范围；
				root->right = pNode2;
			}
			continue;
		}
		if(!findin) 
		{
			leftprevchild.push_back(prev[i+1]);  //
			leftinchild.push_back(in[i]);
		}
		else
		{
			rightprevchild.push_back(prev[i]);
			rightinchild.push_back(in[i]);
		}
	}
	DFS(root->left,leftprevchild,leftinchild);
	DFS(root->right,rightprevchild,rightinchild);
}

Node* buildroot(vector<int> & a,vector<int> &b)
{
	if(a.size() == 0) return NULL;
	Node *pRoot = new Node(a[0]);
    DFS(pRoot,a,b);
    return pRoot;
}
	
int main()
{
#ifdef  LOCAL
	freopen("data.in","r",stdin);
#endif
    char pstr[100];
	char * p = NULL;
	int count = 0;
	vector<int>avec;
	vector<int>bvec;
    while(scanf("%s",pstr)!=EOF)
	{
		p = strtok(pstr,",");
		if(count%2 == 0)
		{
			avec.push_back(p[0]-'0');
			while(p)
			{
				p = strtok(NULL,",");
				avec.push_back(p[0] - '0');
			}
			Node *root = buildroot(avec,bvec);
			connect(root);
		}
		else
		{
			bvec.push_back(p[0]-'0');
			while(p)
			{
				p = strtok(NULL,p);
				bvec.push_back(p[0] - '0');
			}
		}
	}
}

