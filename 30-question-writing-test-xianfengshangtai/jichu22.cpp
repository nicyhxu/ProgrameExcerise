#include<stdio.h>
#include<iostream>
#include<sstream>
using namespace std;

#define FOS_PART(X) 1 /** 代码阅读宏 */
#if FOS_PART("FTYPE STDLIB")
    #define ROS_SIZEOF(X)              sizeof(X)
#endif
typedef struct dynmaic_node
{
   int node_val;
   int node_slot;
   int node_port[2];
}DYNAMIC_NODE_T;

int main()
{
	char*p="abcd";
	char*s="acd";
	//com_len(p,s);
	cout<<strlen(p)<<" "<<strlen(s)<<endl;
	DYNAMIC_NODE_T port_node;
	memset(&port_node,0,sizeof(DYNAMIC_NODE_T));
	port_node.node_port[0] = 123456;
	port_node.node_port[1] = 654321;
	printf("%lld\n",*((long long *)(port_node.node_port)));
	printf("size of long =%d",ROS_SIZEOF(long long));
	return 0;
}