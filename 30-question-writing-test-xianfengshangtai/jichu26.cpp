#include <stdio.h>
#include <stdlib.h>
#include "time.h"

#define INCRESMENT 10
#define INITLIST_SIZE 100

typedef enum 
{
	OVERFLOW = 0,
	ERROR = 1,
	OK = 2,
}status;

typedef struct ElemType
{
	int audio;
	int video;
}ELEM_AV_TYPE;

typedef struct SqList
{
	ELEM_AV_TYPE *av;
    int list_size; 
    int length;
}SQLIST;

typedef struct LNode
{
	 ELEM_AV_TYPE data;
	 struct LNode *next;
}LNode,*LinkList;

//初始化一个数组的节点;
status InitList(SqList & L)
{
	L.av = (ELEM_AV_TYPE *)malloc(sizeof(ELEM_AV_TYPE)*(INITLIST_SIZE));
	if(!L.av) exit(OVERFLOW);
	L.length = 0;
	L.list_size = INITLIST_SIZE;
	return OK;
}

/**
   获取第多少个节点;存在第i个时返回 OK,不存在返回error;
   带有头结点的情况;
*/
status GetElem_L(LinkList L,int i,ELEM_AV_TYPE &e)
{
	LNode *p = NULL;
	int j = 0;
	p = L->next;
	while(j<i && !p)
	{
		p = p->next;
		j++;
	}
	if(!p || j>i) return ERROR;
	e = p->data;
}

/**
   在链表的第i个节点进行删除;
*/
status ListDelete_L(LinkList L, int i,ELEM_AV_TYPE & e)
{
	LNode *p = NULL;
	LNode *q = NULL;
	int j = 0;
	p = L->next;
	while(p && j<i-1) {p = p->next;j++;}
	if(!p || j>i) return ERROR;
	q = p->next;
	p->next = q->next;
	e = q->data; free(q);
	return OK;
}

/**
   在链表的第i个节点插入;
*/
status ListInsert_L(LinkList L, int i,ELEM_AV_TYPE & e)
{
	LNode *p = NULL;
	int j = 0;
	p = L->next;
	while(p && j<i-1) {p = p->next;j++;}
	if(!p || j>i) return ERROR;
	LNode *s  = (LNode *)malloc(sizeof(LNode));
	if(!s) exit(OVERFLOW);
	s->data = e;
	s->next = p->next;
	p->next = s;
	return OK;
}

/**
   在链表的第i个节点插入;
*/
status ListInsert_Header(LinkList L,ELEM_AV_TYPE & e)
{
	LNode *p = NULL;
	int j = 0;
	p = L->next;
	LNode *s  = (LNode *)malloc(sizeof(LNode));
	if(!s) exit(OVERFLOW);
	s->data = e;
	s->next = p;
	L->next = s;
	return OK;
}

/**
   在链表的第i个节点插入;
   初始化链表;
*/
status CreateList_L(LinkList &L)
{
	LNode *p = NULL;
	p = (LNode *)malloc(sizeof(LNode));
	if(!p) exit(OVERFLOW);
	L = p;
	p->next = p;
	return OK;
}

/*
   在i之前插入节点e;next
*/
status InsertList(SqList &list,int i,ELEM_AV_TYPE &e)
{
   if(i<1 && i>list.length + 1)
	   return ERROR;
   if(list.length >= list.list_size)
   {
	   list.av = (ELEM_AV_TYPE *)realloc(list.av,sizeof(ELEM_AV_TYPE)*(list.list_size + INCRESMENT));
	   if(!list.av) exit(OVERFLOW);
	   list.list_size += INCRESMENT;
   } 
   ELEM_AV_TYPE  * p = &list.av[i-1];
   for(ELEM_AV_TYPE *q = &list.av[list.length];q > p;q--)
   {
	   *q = *(q-1);   //注定了有问题
   }
   list.av[i-1] = e;
   ++list.length;
   return  OK;
}

/*
   遍历所有的节点值，并打印出结果;
*/
void Transver(LinkList &L)
{
	LNode *p = L;
	LNode *q = p->next;
	int i = 0;
	while(q != p)
	{
		printf("av Link video of i %d is  %d\n",i,q->data.video);
		printf("av Link video of i %d is  %d\n",i,q->data.audio);
		q = q->next;
	}
}


int main()
{
	SqList L;
	LinkList plink;
	InitList(L);
	printf("length of sqlist %d\n",L.length);
	printf("listsize of sqlist %d\n",L.list_size);
	ELEM_AV_TYPE av_temp;
	av_temp.audio = 0;
	av_temp.video = 1;
	InsertList(L,1,av_temp);
	srand(time(NULL));
	//模拟一个随机数,生成一个随机数值;
    CreateList_L(plink);
	for(int i = 0 ;i<3;i++)
	{
		int j = rand()%10;
		int t = rand()%10;
		ELEM_AV_TYPE avt;
		avt.audio = j;
	    avt.video = t;
		InsertList(L,1,avt);
		ListInsert_Header(plink,avt);
	}
	printf("av video of i %d is  %d\n", 0,L.av[0].video);
	printf("av audio of i %d is  %d\n", 0,L.av[0].audio);
	printf("av video of i %d is  %d\n", 1,L.av[1].video);
	printf("av audio of i %d is  %d\n", 1,L.av[1].audio);
	printf("av video of i %d is  %d\n", 2,L.av[2].video);
	printf("av audio of i %d is  %d\n", 2,L.av[2].audio);
    Transver(plink);
    return 0;
}