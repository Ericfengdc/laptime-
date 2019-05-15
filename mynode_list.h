#pragma once
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
	struct Node *prior;
	struct Node *next;
	float longitude; //经度
	float latitude;  //纬度
	float speed;     //速度
	uint32_t seconds;//unix 时间戳
	uint16_t millisecondes;//毫秒数
	float direction; //方位角，正北为0度
};

//初始化一个链表的节点
Node* create_node(void);

//建立含有N个结点的双向链表
Node* create_list(int n);


//链表的长度
int len_list(Node * node);


//定位到链表的任意位置
Node* pos_list(Node *head, int n);


//正向遍历一个链表
void out_front_list(Node *head);

//反向遍历一个链表
void out_reverse_list(Node *head);

//在链表的头部插入结点
Node* start_insert_list(Node *head);

//将现有节点插入现有链表末尾
void insert_list(Node *head, Node *p);


//在链表的尾部插入结点
Node* end_insert_list(Node *head);


//插入到任意位置之前
Node* insert_befor_list(Node *head, int a);

//插入到任意位置之后 
Node* insert_after_list(Node *head, int a);

//删除头结点
Node* delect_start_list(Node *head);


//删除尾结点
Node* delect_end_list(Node *head);


//删除指定位置的节点
Node* delect_list(Node *head, int i);

//填充数据到节点
void fill_node_once(float dire, float lat, float lng, int time, float speed, Node * head);

//写数据到文件
void write_lap(float timediff);

//两个节点之间的距离
int dis_node(Node* start, Node* end);

//转到后面的第n个节点，若后面没有n个节点，则返回最后一个节点
Node* jump_node_forward(Node* p, int n);

Node* jump_node_back(Node* p, int n);
