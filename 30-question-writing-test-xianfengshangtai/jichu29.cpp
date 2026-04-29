/**
   基本操作集合:
   InitBiTree(&T);  
   DestroyBiTree(&T);
   CreateBiTree(&T,definition);  //销毁二叉树T;
   ClearBiTree(&T);  //将二叉树变为空树;
   BiTreeEmpty(T); BiTreeDepth(T);Root(T);
   Value(T,e);
   Assign(T,&e,value);
   Parent(T,e);LeftChild(T,e);RightChild(T,e);
   LeftSibling(T,e) //返回e的左兄弟;
   RightSibling(T,e) //返回e的右兄弟; 
   PreOrderTraverse(T,visit())  //结点操作的应用函数;
   PostOrderTraverse(T,visit())
   LevelOrderTraverse(T,visit());
**/

#include <stdio.h>
#include <stdlib.h>
#include "time.h"

#define INCRESMENT 10
#define INITLIST_SIZE 100
#define STACK_INIT_SIZE 100   
#define STACKINCRESMENT 10
#define true 1
#define false 0
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

typedef struct BiTNode 
{
	ELEM_AV_TYPE  data;
	struct BiTNode *lchild, *rchild;
}BiTNode,*BiTree;

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

typedef struct SqStack 
{
	BiTree *base;
	BiTree *top;
	int stacksize;
}SqStack;


status InitStack(SqStack &s);
status DestroyStack(SqStack &s);
status ClearStack(SqStack &s);
int StatckEmpty(SqStack &s);
status GetTop(SqStack &S,BiTree & e);
status Push(SqStack &s,BiTree  e);
status Pop(SqStack &s,BiTree &e);

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
	return OK;
}

/**
   在链表的第i个节点进行删除;
*/
status ListDelete_L(LinkList L, int i ,ELEM_AV_TYPE &e)
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
 
		printf("av Link video of i %d is  %d\n",i,q->data.audio);
		q = q->next;
	}
}
int StatckEmpty(SqStack & s)
{
	if(s.top == s.base)
		return true;
	else
		return false;
}

status InitStack(SqStack &s)
{
	s.base = (BiTree *)malloc(STACK_INIT_SIZE * sizeof(BiTree));
	if(NULL == s.base) 
	{
		exit(OVERFLOW);
	}
	else
	{
		s.top = s.base;
		s.stacksize = STACK_INIT_SIZE;
	}
	return OK;
}

status Push(SqStack &s,BiTree e)
{
	printf("s.top %p s.base %p \n",s.top,s.base);
	if(s.top - s.base >= s.stacksize)
	{
		s.base = (BiTree *)realloc(s.base,(STACKINCRESMENT + s.stacksize)* sizeof(BiTree));
		printf(" stack size realloc \n");
		if(NULL == s.base) 
		 return OVERFLOW;
	 	else
		{
			s.stacksize += STACKINCRESMENT;
			s.top = s.base + s.stacksize;
		}
	}
	*s.top++ = e;
	return OK;
}

 status GetTop(SqStack & S,BiTree & e)
{
	if(S.top == S.base) return ERROR;
	e= *(S.top-1);
	return OK;
}

status Pop(SqStack &S,BiTree &e)
{
	if(S.top == S.base) return ERROR;
	e = *--S.top;
	return OK;
}


status DestroyStack(SqStack &s)
{
	if(NULL != s.base)
    free(s.base);
    s.base = NULL;
	s.top = NULL;
	return OK;
}

typedef status (*Visit)(ELEM_AV_TYPE t);   //函数指针的类型名;

status print_audio_video(ELEM_AV_TYPE t)
{
	printf("av video %d av audio %d \n",t.video,t.audio);
	return OK;
}

/**
   先序遍历
   访问根节点，先序遍历左子树，先序遍历右子树，明显是个递归;
   执行Visit函数成功后返回，否则不进行递归;
**/
status PreOrderTraverse(BiTree T, Visit a)
{
	if(T)
	{
		if(a(T->data))
		{
			if(PreOrderTraverse(T->lchild,a))
		       if(PreOrderTraverse(T->rchild,a)) return OK;
		}
		else
		{
			return ERROR;
		}
	}else return OK;
}


status InOrderTraverse(BiTree T,Visit b)
{
	SqStack S;    //初始化栈的信息;
	BiTree atmp;
	InitStack(S);   
	Push(S,T);
	while(!StatckEmpty(S))
	{
       while(GetTop(S,atmp) && atmp) Push(S,atmp->lchild);
	   Pop(S,atmp); //空指针退栈;
	      if(Pop(S,atmp) && atmp) 
		  {
			  if(!b(atmp->data)) return ERROR;
			  Push(S,atmp->rchild);     //加入右子结点;
		  }
	}
	DestroyStack(S);
	return OK;
}

status InOrderTraverse2(BiTree T,Visit b)
{
	SqStack S;    //初始化栈的信息;
	BiTree atmp;
	InitStack(S);   
	atmp = T;
	while(atmp || !StatckEmpty(S))
	{
          if(atmp)
		  {
			  Push(S,atmp);
			  atmp = atmp->lchild;   //空指针不入栈;
		  }
	      else
		  {
			  Pop(S,atmp);
			  if(!b(atmp->data)) return ERROR;
			  atmp = atmp->rchild;   //空指针不入栈;
		  }
	}
	DestroyStack(S);
	return OK;
}

status InOrderTraverse3(BiTree T,Visit b)
{
	if(T)
	{
		if(PreOrderTraverse(T->lchild,b))
		{
		   if(b(T->data))
		   {
			  if(PreOrderTraverse(T->rchild,b)) return OK;
		   }
		   else
		   {
			   return ERROR;
		   }
		}
	}else return OK;
	return OK;
}



status CreateBiTree(BiTree & T)
{
	ELEM_AV_TYPE ch;
	scanf("%d %d",&ch.audio,&ch.video);
	if(255 != ch.audio && ch.video != 255)
    {
		if(!(T = (BiTNode *)malloc(sizeof(BiTNode)))) exit(OVERFLOW);
		T->data = ch;
		CreateBiTree(T->lchild);
		CreateBiTree(T->rchild);
	}
	else
	{
		T = NULL;
	}
	return OK;
}


int main()
{
	SqList L;
	SqStack pstack;
	LinkList plink;
	BiTree bTree;
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
	InitStack(pstack);
	CreateBiTree(bTree);
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
	ELEM_AV_TYPE temp;
	printf("av video of i %d is  %d\n", 0,L.av[0].video);
	printf("av audio of i %d is  %d\n", 0,L.av[0].audio);
	printf("av video of i %d is  %d\n", 1,L.av[1].video);
	printf("av audio of i %d is  %d\n", 1,L.av[1].audio);
	printf("av video of i %d is  %d\n", 2,L.av[2].video);
	printf("av audio of i %d is  %d\n", 2,L.av[2].audio);
    Transver(plink);
	DestroyStack(pstack);
	PreOrderTraverse(bTree,print_audio_video);
	InOrderTraverse(bTree,print_audio_video);
	InOrderTraverse2(bTree,print_audio_video);
	InOrderTraverse3(bTree,print_audio_video);
    return 0;
}