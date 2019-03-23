// test.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
#include "pch.h"
#include <windows.h>
#include <iostream>
#include <math.h>
#include<time.h>
#include "load_data2node.h"
#include "distance.h"
#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))
#define dire_change_variance 3
#define direction_change 2//定义在直道内的允许偏差，超过此值意味着开始转弯，偏离直道
#define collect_length 6
#define dis_limit 0.008

typedef struct
{
	float longitude; //经度
	float latitude;  //纬度
	float speed;     //速度
	uint32_t seconds;//unix 时间戳
	uint16_t millisecondes;//毫秒数
	float direction; //方位角，正北为0度
	uint8_t satellites; //锁定卫星数

}stuGPS;
float max_straight_length = 0;
float start_point_lat = 0;
float start_point_lng = 0;
float start_point_direction = 0;
bool fill_data_in_Node(Node*p,stuGPS stu) //将传过来的信息保存到链表节点中
{
	p->longitude = stu.longitude;
	p->latitude = stu.latitude;
	p->speed = stu.speed;
	p->seconds = stu.seconds;
	p->millisecondes = stu.millisecondes;
	p->direction = stu.direction;
	p->satellites = stu.satellites;
	return false;
}
//找到最长直道的方向角等数据
void get_straight_dire(Node* head) 
{
	Node*p=head;
	Node*max_length_start_node=NULL;
	Node*max_length_end_node=NULL;
	int maxlength=0;
	while (p->next)
	{
		int temp_maxlength = 0;//最长方向角不变的节点长度
		Node*q = p;
		float dire = p->direction;
		float a = q->next->direction;
		while (abs(dire - a) < direction_change)//方向角没动
		{
			q = q->next;
			if (q->next != NULL)
			{
				temp_maxlength++;
				a = q->next->direction;
			}
			else break;
		}
		if (temp_maxlength > maxlength) //当前为最大长度
		{
			maxlength = temp_maxlength;
			max_length_start_node = p;
			max_length_end_node = q;
			start_point_direction = dire;
		}
		p = p->next;
	}
	//根据最长直道端点计算直道长度,起点经纬度，方向角
	max_straight_length = distance(max_length_start_node->latitude,max_length_start_node->longitude,max_length_end_node->latitude,max_length_end_node->longitude);
	start_point_lat = (max_length_start_node->latitude + max_length_end_node->latitude) / 2;
	start_point_lng = (max_length_start_node->longitude + max_length_end_node->longitude) / 2;
	printf("起始点经度：%f，起始点纬度：%f，终点经度:%f，终点纬度:%f,直道长度：%f.1米\n", max_length_start_node->latitude, max_length_start_node->longitude, max_length_end_node->latitude, max_length_end_node->longitude,max_straight_length*1000);
	printf("起点经度：%f  起点纬度：%f  起点方向角：%f\n",start_point_lat,start_point_lng,start_point_direction);
}
//找到最长直道信息后，用此函数找出数据中符合方向角，经纬度要求的起点数据包，后续可能要用此数据包的时间等信息
Node* get_start_point(Node *head) 
{
	int num = 0;
	Node * p = NULL;
	while (head->next != NULL)
	{
		if (abs(head->direction - start_point_direction) < 2) 
		{
		bool dec = head->latitude >= start_point_lat && head->next->latitude < start_point_lat;
		bool inc = head->latitude <= start_point_lat && head->next->latitude > start_point_lat;
		if (dec | inc)
		{
			num++;
			printf("候选点：lat:%f，lng:%f,time:%d\n",head->latitude,head->longitude,head->seconds);
			p = head;
		}
		}
		head = head->next;
	}
	if (num == 1)
	{
		printf("here is the start_point lat:%f,lng:%f,dire:%f\n", p->latitude, p->longitude,p->direction);

		return p;
	}
	else if (num == 0)
	{
		printf("未找到起始节点");
		return NULL;
	}
	else
	{
		printf("存在多个候选节点！，请更改候选节点选择模式 候选节点数：%d",num);
		return NULL;
	}
}
bool iscrossed2(float prior_dis,float next_dis) 
{
	if (prior_dis < next_dis) 
	{
		return true;
	}
	else return false;
}
bool iscrossed(Node *p)//判断当前起点是否越过了真正起点
{
	float last_node_dis = 0, next_node_dis = 0;
	last_node_dis = distance(p->prior->latitude, p->prior->longitude,start_point_lat, start_point_lng);
	next_node_dis = distance(p->next->latitude, p->next->longitude,start_point_lat, start_point_lng);
	if (last_node_dis < next_node_dis) 
	{
		return true;
	}
	else return false;
}
bool is_straight(float dire[],int n) //判断是否是直道
{
	if (variance(dire, n) < dire_change_variance)//方向角变化幅度小，即直道
		return true;
	return false;
}
float setstart_point(stuGPS stu)
{
	if (stu.satellites > 5) //锁定卫星数目大于等于6且当前速度大于设定速度
	{
		//设为起始点
		return stu.direction;
	}

}
//模拟环境，每100ms调用一次，填充数据和链表
void fill_node_once(float dire,float lat,float lng,int time,float speed,Node * head) 
{
	head->direction = dire;
	head->latitude = lat;
	head->longitude = lng;
	head->seconds = time;
	head->speed = speed;
}
//（abandoned）方向是否变化
bool is_dire_change(float old_direction,float direction)
{
	if (abs(old_direction - direction) > direction_change) 
	{
		return true;
	}
	return false;

}
//计算圈时,公式为（t2-delta_t2）-(t1-delata_t1),t1为上圈时间，delta_t1为上一圈插值
float laptime(Node *last_start,Node* this_start) //
{
	float delta_t1=0, delta_t2 = 0,t1=0,t2=0;
	delta_t1 = 1000*distance(last_start->latitude, last_start->longitude, start_point_lat, start_point_lng) / last_start->speed;
	delta_t2 = 1000*distance(this_start->latitude, this_start->longitude, start_point_lat, start_point_lng) / this_start->speed;
	if (!iscrossed(last_start)) //上一个节点是越过的
	{
		delta_t1 = -delta_t1;
	}
	if (!iscrossed(this_start)) 
	{
		delta_t2 = -delta_t2;
	}

	return get_diff_time(this_start->seconds,last_start->seconds)-delta_t2+delta_t1;
}

int main()
{
	stuGPS s;
	float start_dire;//起始点方向角
	float start_speed;//起始点速度
	//float *speed= (float*)malloc(sizeof(float)*collect_length);//寻找起始点时监控速度变化，大小为6，一秒一次
	float *dire= (float*)malloc(sizeof(float)*6);//寻找起始点时监控方向角变化
	float direction[5000];
	float lat[5000];
	float lng[5000];
	int time[5000];
	float speed[5000];
    read_dire2node_once("C:\\Users\\Administrator\\Desktop\\data\\dire_long.txt", direction);
    read_lat2node_once("C:\\Users\\Administrator\\Desktop\\data\\lat_long.txt", lat);
	read_lng2node_once("C:\\Users\\Administrator\\Desktop\\data\\lng_long.txt", lng);
	read_time2node_once("C:\\Users\\Administrator\\Desktop\\data\\time.txt", time);
	read_speed2node_once("C:\\Users\\Administrator\\Desktop\\data\\speed.txt", speed);

	Node*test=create_list(1);

	fill_node_once(direction[0], lat[0], lng[0], time[0],speed[0],test);
	for (int i = 1; i < 470; i++)//预存第一圈数据
	{
		Node*p = create_node();
		fill_node_once(direction[i], lat[i], lng[i],time[i],speed[i],p);
		insert_list(test, p);
	}

	get_straight_dire(test);//找到直道数据
	Node*new_circle=get_start_point(test);//找到后面圈的起点
	start_point_lat = new_circle->latitude;//找到后更新起点位置数据（旧起点没有准确时间）
	start_point_lng = new_circle->longitude;

	for (int i = 470; i < 4726; i++)//插入下一圈
	{
		Node*p = create_node();
		fill_node_once(direction[i], lat[i], lng[i],time[i],speed[i],p);
		//Sleep(1);
		insert_list(test,p);
	}
	Node *p = new_circle->next;
	Node *last_start = new_circle;//记录上一圈的起点节点，用来计算下一圈的时间
	while (p->next != NULL) 
	{
		float dis = distance(new_circle->latitude, new_circle->longitude, p->latitude, p->longitude);
		float next_dis = distance(new_circle->latitude, new_circle->longitude, p->next->latitude, p->next->longitude);
		float prior_dis = distance(new_circle->latitude, new_circle->longitude, p->prior->latitude, p->prior->longitude);
		if (dis < dis_limit&&dis < next_dis&&dis < prior_dis) //距离限度
		{
			printf("距离为：%f  时间是：%d 速度是：%f\n", dis, p->seconds, p->speed);
			printf("前：%f,后：%f,此：%f\n", prior_dis,next_dis,dis);
			printf("本圈时间为：%.2f\n\n",laptime(last_start,p));//计算此圈圈速
			last_start = p;
		}
		p = p->next;
	}
}
