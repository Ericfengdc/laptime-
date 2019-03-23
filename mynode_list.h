#include<stdio.h>
#include <stdlib.h>
#include <string.h>
typedef signed char int8_t;
typedef signed short int  int16_t;
typedef signed int  int32_t;
typedef signed long long int64_t;

typedef unsigned char uint8_t;
typedef unsigned short int  uint16_t;
typedef unsigned int  uint32_t;
typedef unsigned long long uint64_t;


//节点结构体，每个节点分别保存了一个指向前一个节点的指针和指向后一个节点的指针，以及保存数据的指针

//定义双向链表的节点 
typedef struct Node
{
	int data;
	struct Node *prior;
	struct Node *next;
	float longitude; //经度
	float latitude;  //纬度
	float speed;     //速度
	uint32_t seconds;//unix 时间戳
	uint16_t millisecondes;//毫秒数
	float direction; //方位角，正北为0度
	uint8_t satellites; //锁定卫星数
	uint16_t index;
}Node;

//初始化一个链表的节点
Node* create_node(void)
{
	Node *p;
	p = (Node*)malloc(sizeof(Node));
	if (p == NULL)
	{
		printf("动态内存分配失败！\n");
		exit(0);
	}
//	scanf("%d", &(p->data));
	p->prior = NULL;
	p->next = NULL;
	return (p);
}

//建立含有N个结点的双向链表
Node* create_list(int n)
{
	Node *p=NULL, *new1=NULL, *head=NULL;
	int i;
	if (n >= 1)                    //结点的个数 >= 1 的时候，先生成第一个结点 
	{
		new1 = create_node();
		head = new1;
		p = new1;
	}
	for (i = 2; i <= n; i++)    //生成第一个结点以后的结点，并建立双向链表的关系 
	{
		new1 = create_node();
		p->next = new1;
		new1->prior = p;
		p = new1;
	}
	if (n >= 1)
		return (head);
	else
		return 0;
}



//链表的长度
int len_list(Node * node)
{
	Node*p = node;
	int len=0;
	while (p != NULL)
	{
		p = p->next;
		len++;
	}
	return len;
}

//定位到链表的任意位置
Node* pos_list(Node *head, int n)
{
	int i = 1;
	Node *p=NULL;
	if (i <= n)
	{
		p = head;
		for (i = 2; i <= n; i++)
			p = p->next;
	}
	return p;
}

//正向遍历一个链表
void out_front_list(Node *head)
{
	if (head == NULL)
	{
		printf("输入的链表信息有误，链表不存在！\n");
	}
	else
	{
		Node *p;
		p = head;
		while (p != NULL)
		{
			printf("%d  ", p->data);
			p = p->next;
		}
	}
}

//反向遍历一个链表
void out_reverse_list(Node *head)
{
	if (head == NULL)
	{
		printf("输入的链表信息有误，链表不存在！\n");
	}
	else
	{
		int n;
		n = len_list(head);
		Node *p;
		p = pos_list(head, n);
		while (p != NULL)
		{
			printf("%d  ", p->data);
			p = p->prior;
		}
	}
}

//在链表的头部插入结点
Node* start_insert_list(Node *head)
{
	Node *p;
	p = create_node();
	p->next = head;
	head->prior = p;
	head = p;
	return (p);
}
//将现有节点插入现有链表末尾
void insert_list(Node *head,Node *p)
{
	int n;
	n = len_list(head);
	head = pos_list(head, n);
	p->prior = head;
	head->next = p;
}

//在链表的尾部插入结点
Node* end_insert_list(Node *head)
{
	int n;
	n = len_list(head);
	Node *p, *new1;
	new1 = create_node();
	p = pos_list(head, n);
	p->next = new1;
	new1->prior = p;
	return (head);
}

//插入到任意位置之前
Node* insert_befor_list(Node *head,int a)
{
	int  newlen;
	Node *pos, *p;
	newlen = len_list(head);
	if (a > newlen)
	{
		head = end_insert_list(head);
	}
	else
	{
		if (a <= 1)
		{
			head = start_insert_list(head);
		}
		else
		{
			pos = pos_list(head, a);
			p = create_node();
			pos->prior->next = p;
			p->prior = pos->prior;
			p->next = pos;
			pos->prior = p;
		}
	}
	return (head);
}

//插入到任意位置之后 
Node* insert_after_list(Node *head,int a)
{
	int  newlen;
	Node *pos, *p;
	newlen = len_list(head);
	if (a >= newlen)
	{
		head = end_insert_list(head);
	}
	else
	{
		if (a < 1)
		{
			head = start_insert_list(head);
		}
		else
		{
			pos = pos_list(head, a);
			p = create_node();
			p->next = pos->next;
			pos->next->prior = p;
			pos->next = p;
			p->prior = pos;
		}
	}
	return (head);
}
//删除头结点
Node* delect_start_list(Node *head)
{
	Node *pos;
	pos = head;
	head = head->next;
	head->prior = NULL;
	free(pos);
	return(head);
}

//删除尾结点
Node* delect_end_list(Node *head)
{
	Node *p, *pos;
	int newlen;
	newlen = len_list(head);
	pos = pos_list(head, newlen);
	p = pos;
	p = p->prior;
	p->next = NULL;
	free(pos);
	return (head);
}

//删除指定位置的节点
Node* delect_list(Node *head,int i)
{
	int newlen;
	Node *pos;
	newlen = len_list(head);
	if (i <= 1)
		head = delect_start_list(head);
	else if (i >= newlen)
		head = delect_end_list(head);
	else
	{
		pos = pos_list(head, i);
		pos->prior->next = pos->next;
		pos->next->prior = pos->prior;
		free(pos);
	}
	return(head);
}
