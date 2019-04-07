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
#define enumber 12345
#define time_find_start 500

float max_straight_length = enumber;
float start_point_lat = enumber;
float start_point_lng = enumber;
float start_point_direction = enumber;
float fastest_speed[500];//最快圈的速度集合
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
	if (head == NULL) 
	{
		printf("ERROR in get_start_point：参数节点为空");
		return NULL;
	}
	int num = 0;
	Node * p = NULL;
	while (head->next != NULL)
	{
		if (abs(head->direction - start_point_direction) < 2) //方向角差距小于2
		{
			bool dec = head->latitude >= start_point_lat && head->next->latitude < start_point_lat;//候选点的纬度和next的纬度包含起点纬度
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
//判断当前起点是否越过了真正起点
bool iscrossed(Node *p)
{
	if (p == NULL)
	{
		printf("ERROR in iscrossed：参数节点为空");
		return NULL;
	}
	float last_node_dis = 0, next_node_dis = 0;
	last_node_dis = distance(p->prior->latitude, p->prior->longitude,start_point_lat, start_point_lng);
	next_node_dis = distance(p->next->latitude, p->next->longitude,start_point_lat, start_point_lng);
	if (last_node_dis < next_node_dis) 
	{
		return true;
	}
	else return false;
}
//判断是否是直道
bool is_straight(float dire[],int n) 
{
	if (variance(dire, n) < dire_change_variance)//方向角变化幅度小，即直道
		return true;
	return false;
}
//模拟环境，每100ms调用一次，填充数据和链表
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
	if (last_start == NULL||this_start == NULL)
	{
		printf("ERROR in iscrossed：参数节点为空");
		return 0;
	}
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
//判断当前节点是不是离起点距离最短的
//abandoned(三次计算distance吃不消)
bool is_start(Node*p,Node*last_start) 
{
	if (p->next == NULL) 
	{
		printf("ERROR:计算距离差时节点后继为空");
		return false;
	}
	if (last_start == NULL) 
	{
		printf("ERROR:计算距离差时上个起点为空");
	}
	float dis = distance(start_point_lat, start_point_lng, p->latitude, p->longitude);
	float next_dis = distance(start_point_lat, start_point_lng, p->next->latitude, p->next->longitude);
	float prior_dis = distance(start_point_lat, start_point_lng, p->prior->latitude, p->prior->longitude);
	if (dis < dis_limit&&dis < next_dis&&dis < prior_dis) //距离限度
	{
		printf("距离为：%f  时间是：%d 速度是：%f\n", dis, p->seconds, p->speed);
		printf("前：%f,后：%f,此：%f\n", prior_dis, next_dis, dis);
		printf("本圈时间为：%.2f\n", laptime(last_start, p));//计算此圈圈速
		return true;
	}
	return false;
}
float sum_dis(Node*a,Node*b) 
{
	float sum=0;
	if (a == NULL||b == NULL) 
	{
		printf("ERROR in sun_dis:参数为空");
		return 0;
	}
	while (a != b) 
	{
		sum += distance(a->latitude,a->longitude,a->next->latitude,a->next->longitude);
		a = a->next;
	}
	return sum;
}
//更新最快圈数据
float* load_fast(Node*start,Node*last,float fast[500])
{
	int i=0;

	while (start->next != last) 
	{
		fast[i] = start->speed;
		i++;
		start = start->next;
	}
	//将最后所有数据填充为最后一个速度，方便之后实时计算时间
	int last_index = i-1;
	for (i; i < 500; i++)
	{
		fast[i] = fast[last_index];
	}
	return fast;
}
float get_timediff(float speed_prior,float speed_next,int index)//获取实时时间差
{
	float speed = 0.5 * (speed_next + speed_prior);//平均速度
	float fast_mi = (fastest_speed[index] + fastest_speed[index + 1]) / 2;
	float result=(0.1*(1-fast_mi/speed));
	printf("index:%d 最快速度：%f,当前速度：%f,增量：%f", index,fastest_speed[index], speed,result);
	//printf("时间差 %f\n", result);
	return result;
}
int main()
{
	float start_dire;//起始点方向角
	float start_speed;//起始点速度
	//float *speed= (float*)malloc(sizeof(float)*collect_length);//寻找起始点时监控速度变化，大小为6，一秒一次
	float *dire= (float*)malloc(sizeof(float)*6);//寻找起始点时监控方向角变化
	float direction[5000];
	float lat[5000];
	float lng[5000];
	int time[5000];
	float speed[5000];
	int index=0,fast_index=0,timediff_index=0;//节点索引(测试用)，最快圈索引，计算时间差用的索引
    read_dire2node_once("C:\\Users\\Administrator\\Desktop\\data\\dire_long.txt", direction);
    read_lat2node_once("C:\\Users\\Administrator\\Desktop\\data\\lat_long.txt", lat);
	read_lng2node_once("C:\\Users\\Administrator\\Desktop\\data\\lng_long.txt", lng);
	read_time2node_once("C:\\Users\\Administrator\\Desktop\\data\\time.txt", time);
	read_speed2node_once("C:\\Users\\Administrator\\Desktop\\data\\speed.txt", speed);

	Node*test=create_list(1);
	Node*new_circle = NULL;
	Node*p = NULL;//计算时间时指向当前节点
	Node*last_start = NULL;//指向上一次的起点
	float dis = 0, next_dis = 0, prior_dis = 0,minlaptime= enumber,time_diff=0;
	while (index<4724)
	{
		//每次进入先新建节点
		Node*new_node = create_node();
		//Sleep(100);
		fill_node_once(direction[index], lat[index], lng[index], time[index], speed[index], new_node);//调用小尘接口（放在参数里）
		insert_list(test, new_node);
		//当50秒之后开始找起点
		if(len_list(test)>=time_find_start&&start_point_lat==enumber)
		{
			get_straight_dire(test);//找到直道数据
			new_circle = get_start_point(test);//找到后面圈的起点
			start_point_lat = new_circle->latitude;//找到后更新起点位置数据（旧起点是平均值，没有准确时间）
			start_point_lng = new_circle->longitude;
		}
		if(new_circle!=NULL)
		{	
			//首次初始化,完成后在new_circle和第500个节点之间试图寻找一圈终点
			if (p == NULL)
			{
				p = new_circle->next; 

				last_start = new_circle;//记录上一圈的起点节点，用来计算下一圈的时间

				//在new_circle和第500个节点之间试图寻找一圈终点
				while (p->next != NULL)
				{
					//dis们的更新
					prior_dis = dis;
					dis = next_dis;
					next_dis = distance(start_point_lat, start_point_lng, p->next->latitude, p->next->longitude);
					//距离限度
					if (dis < dis_limit&&dis < next_dis&&dis < prior_dis) 
					{
						printf("距离为：%f  时间是：%d 速度是：%f\n", dis, p->seconds, p->speed);
						printf("前：%f,后：%f,此：%f\n", prior_dis, next_dis, dis);
						printf("本圈时间为：%.2f\n", laptime(last_start, p));//计算此圈圈速
						last_start = p;//上一个起点的更新
					}
					p = p->next;	
					fast_index++;
				}
			}

			p = new_node->prior;//p指向最新节点的前一个，因为需要与后一个节点判断距离差（assert p->next!=NULL）,一开始p是第500个node
			//dis们的更新
			prior_dis = dis;
			dis = next_dis;
			next_dis = distance(start_point_lat, start_point_lng, new_node->latitude, new_node->longitude);;

			if (dis < dis_limit&&dis < next_dis&&dis < prior_dis) //距离限度
			{
				float lap_time = laptime(last_start, p);
				printf("距离为：%f  时间是：%d 速度是：%f\n", dis, p->seconds, p->speed);
				printf("前：%f,后：%f,此：%f\n", prior_dis, next_dis, dis);
				printf("本圈时间为：%.2f\n", lap_time);//计算此圈圈速
				printf("本圈距离：%.3f km\n", sum_dis(last_start, p));
				//更新最快圈
				if (lap_time < minlaptime)
				{
					minlaptime = lap_time;
					load_fast(last_start, p, fastest_speed);//更新
				}
				last_start = p;//上一个起点的更新
				timediff_index = 0;//新的一圈时，实时时间差索引,时间差归零。
				time_diff = 0;
			}
			time_diff += get_timediff(p->speed,p->next->speed,timediff_index);
			p = p->next;
			//开始计算实时时间差
			
			timediff_index++;
			printf("最快圈时间为：%f 与最快圈差距%.2f\n",minlaptime,time_diff);
		}
		index++;
	}
}
