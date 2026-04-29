#include<stdio.h>
#include<iostream>
#include<sstream>

using namespace std;

typedef struct tag_single_linked_list_node
{
    struct tag_single_linked_list_node *next;    /*!< 后继节点的指针 */
}UDS_SLL_NODE_INTRA;

#define UDS_SLL_LIST_T                                 UDS_SLL_LIST_INTRA
/**
 *\brief 单向链表的定义
 */
typedef struct tag_single_linked_list
{
    UDS_SLL_NODE_INTRA *first;         /*!< 单向链表的第一个节点的指针 */
    UDS_SLL_NODE_INTRA *last;          /*!< 单向链表的最后一个节点指针 */
}UDS_SLL_LIST_INTRA;

typedef struct tag_hafss_port_base 
{
    UDS_SLL_LIST_T hafss_port_list; /* 数据链接端口链表*/
    int all_port_num;
    int port_num;
}HAFSS_PORT_BASE;

typedef struct port_node
{
	UDS_SLL_NODE_INTRA * ptr;
	int port;
}dynamic_port_node;

 /**
 *\brief lib句柄结构体
 */

typedef struct tag_hafssl_global
{
    HAFSS_PORT_BASE *port_sour;             /* 端口号资源池 */
}HAFSSL_GLOBAL;

HAFSSL_GLOBAL *g_hafssl_ser = NULL;

typedef struct hafssl_global
{
    void *port_sour;             /* 端口号资源池 */
}SSL_GLOBAL;

int main()
{
	char command[500]= "";
	int append_delta = 0;
	sprintf(command,"mod CGM_HAPM 0 15 HEADER_APPEND_DELTA=%d \n",append_delta);
	cout<<command<<endl;
	g_hafssl_ser = (HAFSSL_GLOBAL*)malloc(sizeof(HAFSSL_GLOBAL));
	dynamic_port_node*pnode = (dynamic_port_node*)malloc(sizeof(dynamic_port_node));
	cout<<pnode<<endl;
	memset(pnode, 0 , sizeof(dynamic_port_node));
	pnode->port = 10;
	cout<<"port_num"<<pnode->port<<endl;
	memset(g_hafssl_ser, 0 , sizeof(HAFSSL_GLOBAL));
	g_hafssl_ser->port_sour = (HAFSS_PORT_BASE*)malloc(sizeof(HAFSS_PORT_BASE));
	g_hafssl_ser->port_sour->hafss_port_list.first = NULL;
	g_hafssl_ser->port_sour->hafss_port_list.last = NULL;
	do 
    {
        ((UDS_SLL_NODE_INTRA *)pnode)->next = NULL;
        if ((g_hafssl_ser->port_sour->hafss_port_list).last != NULL)                                              
        {                                                                     
            (g_hafssl_ser->port_sour->hafss_port_list).last->next = (UDS_SLL_NODE_INTRA *)(pnode);                           
        }                                                                     
        else                                                                  
        {                                                                     
            (g_hafssl_ser->port_sour->hafss_port_list).first = (UDS_SLL_NODE_INTRA *)(pnode); 
            cout<<"1"<<(g_hafssl_ser->port_sour->hafss_port_list).first<<endl;
			cout<<"1"<<((dynamic_port_node*)(g_hafssl_ser->port_sour->hafss_port_list).first)->port<<endl; 
        } 
        (g_hafssl_ser->port_sour->hafss_port_list).last = (UDS_SLL_NODE_INTRA *)(pnode);                                     
    }while(0);
	cout<<"first ptr"<<((dynamic_port_node*)(g_hafssl_ser->port_sour->hafss_port_list).first)->port<<endl;
	HAFSSL_GLOBAL * a = (HAFSSL_GLOBAL*)malloc(sizeof(HAFSSL_GLOBAL));
	SSL_GLOBAL *b = (SSL_GLOBAL*)a+1;
	SSL_GLOBAL*c = (SSL_GLOBAL*)(a + 1);
	cout<<a<<endl;
	cout<<b<<endl;
	cout<<c<<endl;
    return 0;
}