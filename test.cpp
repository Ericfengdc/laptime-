//😂😂😂😂😂（这表情也可以放这？？？）
#pragma once
#include "pch.h"
#include <windows.h>
#include <iostream>
#include <math.h>
#include <time.h>
#include "load_data2node.h"
#include "distance.h"
#define direction_change 2                          //定义在直道内的允许偏差，超过此值意味着开始转弯，偏离直道			
#define dis_limit 0.005								//在计算是否到达终点时，首先两点之间距离至少要小于5米
#define enumber 12345                               //错误数据，初始化时使用
#define time_find_start  500                        //在收到500个节点之后才开始在其中寻找直道和起点
float max_straight_length                       = enumber;
float start_point_lat                           = enumber;
float start_point_lng                           = enumber;
float start_point_direction                     = enumber;
int lap_count                                   = 0;
int time_compare_node_index                     = 0;//记录当前节点在本圈中的序号，在计算时间差时用来指示当前搜索到了最快圈的哪个节点
Node*fast_start                                 = NULL, *fast_end = NULL;
float fastest_lap_speed[500];                       //最快圈的速度集合
float fastest_lap_lat[500];
float fastest_lap_lng[500];
float fastest_lap_dis[500];
int fastest_lap_time[500];
float real_segtime[500]                         = { 0 };
int real_segtime_index                          = 0;//计算每次与最快圈差距时的实际时间差，显示时为滤波时间


int main()
{
	float direction[5000];
	float lat[5000];
	float lng[5000];
	int time[5000];
	float speed[5000];
	int index           = 0, fast_index = 0, start_time_lap = 0;   //节点索引，最快圈索引,本圈起始时间
	float dis           = 0, next_dis = 0, prior_dis = 0, minlaptime = enumber;
	float mileage       = 0, seg_mileage = 0;                      //当前圈里程和当前圈当前段里程  
	float last_time_diff = 0;				       //上次时间差，用来滤波
	Node*lap            = create_list(1);                          //新建节点为整个过程起点
	Node*new_circle     = NULL;                                    //保存第一圈起点
	Node*p              = NULL;                                    //计算时间时指向当前节点
	Node*last_start     = NULL;                                    //指向上一次的起点

	                                                                                                  
	load_data(direction, lat, lng, time, speed);													  //加载模拟数据
	while (index < 4724)
	{
		                                                                                              //每次进入先新建节点
		Node*new_node = create_node();
		/*调用小尘接口（放在参数里）*/
		fill_node_once(direction[index], lat[index], lng[index], time[index], speed[index], new_node);
		if (start_point_lat == enumber && index % 10 != 0)                                            //在第一圈开始时每次只一秒记录一个值
		{
			index++;
			insert_list(lap, new_node);
			continue;
		}
		insert_list(lap, new_node);
		/*当50秒之后开始找起点
		if (len_list(lap) >= time_find_start && start_point_lat == enumber)
		{
			get_straight_dire(lap);                                                                   //找到直道数据
			new_circle = get_start_point(lap);                                                        //找到后面圈的起点
			set_start(new_circle);                                                                    //设置起点信息，当前圈数
			start_time_lap = new_circle->seconds;                                                     //当前圈起始时间
		}
		*/
		/*寻找每圈的闭环，用新点和旧的点进行比较*/
		if (new_node != NULL && start_point_lat == enumber && len_list(lap)>200 )
		{
			Node* q = lap;
			Node* now_node = new_node->prior;
			Node* last_node2compare = jump_node_back(now_node,100);
			while (q != last_node2compare) 
			{
				if (abs(q->direction - now_node->direction) < 4 && distance(q->latitude, q->longitude, now_node->latitude, now_node->longitude) < 0.005)
				{
					new_circle = now_node;
					set_start(now_node);
					start_time_lap = now_node->seconds;
					printf("start_time:%d,%d",start_time_lap,q->seconds);
					break;
				}
					q = jump_node_forward(q,10);
			}

		}
		/*以速度值找起点
		if (new_node->prior!=NULL && new_node->prior->speed > 90 && start_point_lat == enumber) 
		{
			new_circle = new_node->prior;
			set_start(new_circle);
			start_time_lap = new_circle->seconds;
		}
		*/
		if (new_circle)
		{
			/*首次初始化,完成后在new_circle和第500个节点之间试图寻找一圈终点*/
			if (!p)
			{
				p = new_circle->next;
				//记录上一圈的起点节点，用来计算下一圈的时间
				last_start = new_circle;
				/*在new_circle和第500个节点之间试图寻找一圈终点*/
				while (p!=NULL && p->next != NULL)
				{
					//上段距离，这里计算上段距离有原因，因为下面p = new_node->prior之后调整过来计算当前段了
					update_mileage(seg_mileage, mileage, p->prior);
					//dis们的更新
					update_dis(prior_dis, dis, next_dis, p->next);
					//距离限度，在限度内认为是起点
					if (dis < dis_limit&&dis < next_dis&&dis < prior_dis)
					{
						printf("距离为：%f  时间是：%d 速度是：%f\n", dis, p->seconds, p->speed);
						printf("本圈时间为：%.2f 本圈距离：%.3f km\n", laptime(last_start, p), mileage);//计算此圈圈速
						last_start = p;                                                      //上一个起点的更新
						start_time_lap = p->seconds;                                         //更新起始时间
					}
					p = p->next;
					fast_index++;
				}
				/*试图寻找结束*/
			}

			//在这之前从上一个if出来p==new_node,需要调整
			p = new_node->prior;//p指向最新节点的前一个，因为需要与后一个节点判断距离差（assert p->next!=NULL）,一开始p是第500个node
			//新节点更新里程
			update_mileage(seg_mileage, mileage, p);
			//dis们的更新,寻找离起点最近的点
			update_dis(prior_dis, dis, next_dis, new_node);
			//打印当前点相对最快圈时间差距
			//get_timediff(mileage,start_time_lap,p->seconds);
			last_time_diff = get_timediff1(p->latitude, p->longitude, start_time_lap, p->seconds, time_compare_node_index, last_time_diff);

			if (dis < dis_limit&&dis < next_dis&&dis < prior_dis) //距离限度
			{
				if (fast_end != NULL)
				{
					//compare_time(fast_start, fast_end, last_start, p);
				}
				float lap_time = laptime(last_start, p);

				printf("最终领先时间%.2f\n", minlaptime - lap_time);
				/*更新最快圈信息*/
				update_fast_lap(lap_time, minlaptime, last_start, p);

				printf("本圈时间为：%.2f\n", lap_time);//打印此圈时间
				printf("本圈距离：%.3f km  dis:%f  \n", mileage, distance(p->latitude, p->longitude, start_point_lat, start_point_lng));

				/*新的一圈清零处*/
				lap_count++;                //圈数加一
				last_start = p;             //上一个起点的更新
				start_time_lap = p->seconds;//
				mileage = 0;                //总里程清零
				time_compare_node_index = 0;
				                            //恢复记录真实时间差
				for (int i = 0; i < 500; i++)
					real_segtime[i] = 0;
				real_segtime_index = 0;
				last_time_diff = 0;
			}
			p = p->next;
								            //开始计算实时时间差
		}
		index++;
	}
}

/*distance中函数的定义*/


double mysin(double x)
{
	float y = B * x + C * x * abs(x);
	y = P * (y * abs(y) - y) + y;
	return y;
}
double mycos(double x)
{
	return mysin(x + PI / 2);
}
//计算距离
double get_distance(double lat1, double lng1, double lat2, double lng2)
{
	double radLat1 = radian(lat1);
	double radLat2 = radian(lat2);
	double a = radLat1 - radLat2;
	double b = radian(lng1) - radian(lng2);
	double dst = 2 * asin((sqrt(pow(mysin(a / 2), 2) + mycos(radLat1) * mycos(radLat2) * pow(mysin(b / 2), 2))));
	//double dst = 2 * asin((sqrt(pow(sin(a / 2), 2) + cos(radLat1) * cos(radLat2) * pow(sin(b / 2), 2))));

	dst = dst * EARTH_RADIUS;
	dst = round(dst * 10000) / 10000;
	return dst;
}
double distance(double lat1, double lng1, double lat2, double lng2)
{

	double dst = get_distance(lat1, lng1, lat2, lng2);
	return dst;
}
float get_diff_time(int a, int b) //a大b小
{
	int minute = a / 1000 - b / 1000;
	int second = a / 10 % 100 - b / 10 % 100;
	int msecond = a % 10 - b % 10;
	if (second < 0)
	{
		second = second + 60;
		minute -= 1;
	}
	if (msecond < 0)
	{
		msecond = msecond + 10;
		second -= 1;
	}
	float time = float(minute * 60) + float(second) + float(msecond) / 10;
	return time;
}
/*load_data2node.h函数实现*/
float* read_dire2node(const char *path, Node *head)
{
	FILE *f;
	fopen_s(&f, path, "r");
	Node *p = head;
	if (f != NULL)
	{
		while (p != NULL)
		{
			fscanf_s(f, "%f", &p->direction, sizeof(float));
			//printf("%f\n", p->direction);
			p = p->next;
		}
		fclose(f);
		return 0;
	}
	else printf("dire文件打开失败");
	return 0;
}
float* read_lat2node(const char *path, Node *head)
{
	FILE *f;
	fopen_s(&f, path, "r");
	Node *p = head;
	if (f != NULL)
	{
		while (p != NULL)
		{
			fscanf_s(f, "%f", &p->latitude, sizeof(float));
			//printf("%f\n", p->direction);
			p = p->next;
		}
		fclose(f);
		return 0;
	}
	else printf("lat文件打开失败");
	return 0;
}
float* read_lng2node(const char *path, Node *head)
{
	FILE *f;
	fopen_s(&f, path, "r");
	Node *p = head;
	if (f != NULL)
	{
		while (p != NULL)
		{
			fscanf_s(f, "%f", &p->longitude, sizeof(float));
			//printf("%f\n", p->direction);
			p = p->next;
		}
		fclose(f);
		return 0;
	}
	else printf("lng文件打开失败");
	return 0;
}
void load_data(const char*dire_path, const char* lat_path, const char*lng_path, Node*p)
{
	read_dire2node(dire_path, p);
	read_lat2node(lat_path, p);
	read_lng2node(lng_path, p);
}
/*一次只读一个*/

float* read_dire2node_once(const char*path, float dire[5000])
{
	FILE *f;
	int index = 0;
	fopen_s(&f, path, "r");
	if (f != NULL)
	{
		while (EOF != fscanf_s(f, "%f", &dire[index], sizeof(float)))
		{
			index++;
		}
		fclose(f);
		return dire;
	}
	else printf("dire文件打开失败");
	return dire;
}
float* read_lat2node_once(const char*path, float lat[5000])
{
	FILE *f;
	int index = 0;
	fopen_s(&f, path, "r");
	if (f != NULL)
	{
		while (EOF != fscanf_s(f, "%f", &lat[index], sizeof(float)))
		{
			index++;
		}
		fclose(f);
		return lat;
	}
	else printf("dire文件打开失败");
	return lat;
}
float* read_lng2node_once(const char*path, float lng[5000])
{
	FILE *f;
	int index = 0;
	fopen_s(&f, path, "r");
	if (f != NULL)
	{
		while (EOF != fscanf_s(f, "%f", &lng[index], sizeof(float)))
		{
			index++;
		}
		fclose(f);
		return lng;
	}
	else printf("dire文件打开失败");
	return lng;
}

int* read_time2node_once(const char*path, int time[5000])
{
	FILE *f;
	int index = 0;
	fopen_s(&f, path, "r");
	if (f != NULL)
	{
		while (EOF != fscanf_s(f, "%d", &time[index], sizeof(int)))
		{
			index++;
		}
		fclose(f);
		return time;
	}
	else printf("time文件打开失败");
	return time;
}
void read_speed2node_once(const char*path, float speed[5000])
{
	FILE *f;
	int index = 0;
	fopen_s(&f, path, "r");
	if (f != NULL)
	{
		while (EOF != fscanf_s(f, "%f", &speed[index], sizeof(float)))
		{
			index++;
		}
		fclose(f);
	}
	else printf("speed文件打开失败");
}
/*my_node_list的实现*/
Node* create_node(void)
{
	Node *p;
	p = (Node*)malloc(sizeof(Node));
	if (p == NULL)
	{
		printf("动态内存分配失败！\n");
		exit(0);
	}
	p->prior = NULL;
	p->next = NULL;
	return (p);
}

//建立含有N个结点的双向链表
Node* create_list(int n)
{
	Node *p = NULL, *new1 = NULL, *head = NULL;
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
	int len = 0;
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
	Node *p = NULL;
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
			printf("%d  ", p->seconds);
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
			printf("%d  ", p->seconds);
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
void insert_list(Node *head, Node *p)
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
Node* insert_befor_list(Node *head, int a)
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
Node* insert_after_list(Node *head, int a)
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
Node* delect_list(Node *head, int i)
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
//填充数据到节点
void fill_node_once(float dire, float lat, float lng, int time, float speed, Node * head)
{
	head->direction = dire;
	head->latitude = lat;
	head->longitude = lng;
	head->seconds = time;
	head->speed = speed;
}
void write_lap(float timediff)
{
	FILE*f = fopen("C:\\Users\\Administrator\\Desktop\\2.txt", "a");
	fprintf(f, "%.2f\n", timediff);
	fclose(f);
}
int dis_node(Node* start, Node* end) 
{
	if (start == NULL || end == NULL) 
	{
		printf("求节点之间距离是参数为NULL");
		return 0;
	}
	int dis = 0;
	while (start != end) 
	{
		dis++;
		start = start->next;
		if (dis > 1000000) 
		{
			printf("节点之间距离过大");
			return 1000000;
		}
		return dis;
	}
}
Node* jump_node_forward(Node* p, int n)
{
	for (int i = 0; i < n; i++)
	{
		if (p->next != NULL)
			p = p->next;
		else return p;
	}
	return p;
}
Node* jump_node_back(Node * p, int n)
{
	for (int i = 0; i < n; i++)
	{
		if (p->prior != NULL)
			p = p->prior;
		else return p;
	}
	return p;
}

/*其他函数*/
void get_straight_dire(Node* head)
{
	Node*p                     = head;
	Node*max_length_start_node = NULL;//最长直道起始节点
	Node*max_length_end_node   = NULL;//最长直道末尾节点
	int maxlength              = 0;   //最长直道长度
	while (p->next)
	{
		int temp_maxlength = 0;                 //最长方向角不变的节点长度
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
		if (temp_maxlength > maxlength)         //当前为最大长度
		{
			maxlength = temp_maxlength;
			max_length_start_node = p;
			max_length_end_node = q;
			start_point_direction = dire;
		}
		p = p->next;
	}
	//根据最长直道端点计算直道长度,起点经纬度，方向角
	max_straight_length = distance(max_length_start_node->latitude, max_length_start_node->longitude, max_length_end_node->latitude, max_length_end_node->longitude);
	start_point_lat = (max_length_start_node->latitude + max_length_end_node->latitude) / 2;
	start_point_lng = (max_length_start_node->longitude + max_length_end_node->longitude) / 2;
}

//找到最长直道信息后，用此函数找出数据中符合方向角，经纬度要求的起点节点，后续可能要用此节点的时间等信息
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
				printf("候选点：lat:%f，lng:%f,time:%d\n", head->latitude, head->longitude, head->seconds);
				p = head;
			}
		}
		head = head->next;
	}
	if (num == 1)
	{
		printf("起点 lat:%f,lng:%f,dire:%f\n", p->latitude, p->longitude, p->direction);
		return p;
	}
	else if (num == 0)
	{
		printf("未找到起始节点");
		return NULL;
	}
	else
	{
		printf("存在多个候选节点！，请更改候选节点选择模式 候选节点数：%d", num);
		return NULL;
	}
}
/*判断当前起点是否越过了真正起点*/
bool iscrossed(Node *p)
{
	if (p == NULL)
	{
		printf("ERROR in iscrossed：参数节点为空");
		return NULL;
	}
	float last_node_dis = 0, next_node_dis = 0;
	last_node_dis = distance(p->prior->latitude, p->prior->longitude, start_point_lat, start_point_lng);
	next_node_dis = distance(p->next->latitude, p->next->longitude, start_point_lat, start_point_lng);
	if (last_node_dis < next_node_dis)
	{
		return true;
	}
	else return false;
}
/*计算圈时,公式为（t2-delta_t2）-(t1-delata_t1),t1为上圈时间，delta_t1为上一圈插值*/
float laptime(Node *last_start, Node* this_start) //
{
	if (last_start == NULL || this_start == NULL)
	{
		printf("ERROR in laptime：参数节点为空");
		return 0;
	}
	float delta_t1 = 0, delta_t2 = 0, t1 = 0, t2 = 0;
	delta_t1 = 1000 * distance(last_start->latitude, last_start->longitude, start_point_lat, start_point_lng) / last_start->speed;
	delta_t2 = 1000 * distance(this_start->latitude, this_start->longitude, start_point_lat, start_point_lng) / this_start->speed;
	if (!iscrossed(last_start)) //上一个节点是没越过的
	{
		delta_t1 = -delta_t1;
	}
	if (!iscrossed(this_start)) //本次节点未越过起点
	{
		delta_t2 = -delta_t2;
	}
	printf("插值时间%f(-)", delta_t2 - delta_t1);
	return get_diff_time(this_start->seconds, last_start->seconds) - delta_t2 + delta_t1;
}
/*判断当前节点是不是离起点距离最短的*/
void set_start(Node*new_circle)
{
	start_point_lat = new_circle->latitude;//找到后更新起点位置数据（旧起点是平均值，没有准确时间）
	start_point_lng = new_circle->longitude;
	lap_count = 1;//当前为第一圈
}
void update_dis(float&prior_dis, float&dis, float&next_dis, Node*p)
{
	prior_dis = dis;
	dis = next_dis;
	next_dis = distance(start_point_lat, start_point_lng, p->latitude, p->longitude);
}
float sum_dis(Node*a, Node*b)
{
	float sum = 0;
	if (a == NULL || b == NULL)
	{
		printf("ERROR in sun_dis:参数为空");
		return 0;
	}
	while (a != b)
	{
		sum += distance(a->latitude, a->longitude, a->next->latitude, a->next->longitude);
		a = a->next;
	}
	return sum;
}
void update_mileage(float&seg_mileage, float&mileage, Node *p)
{
	seg_mileage = distance(p->latitude, p->longitude, p->next->latitude, p->next->longitude);
	//总里程
	mileage += seg_mileage;
}
/*更新最快圈数据，包含加载数据，距离，时间，已经整体更新*/
float* load_fast_speed(Node*start, Node*last, float fast[500], float lat[500], float lng[500])
{
	int i = 0;

	while (start->prior != last)
	{
		fast[i] = start->speed;
		lat[i] = start->latitude;
		lng[i] = start->longitude;
		i++;
		start = start->next;
	}
	/*将最后所有数据填充为最后一个速度，方便之后实时计算时间*/
	int last_index = i - 1;
	for (i; i < 500; i++)
	{
		fast[i] = fast[last_index];
		lat[i] = enumber;
		lng[i] = enumber;
	}
	return fast;
}

/*最快圈时储存每两个节点之间的距离，后期如果优化计算负载可以在此函数内不用distance计算
  因为在计算总里程的时候更新过,但需要额外的空间来保存dis变量，以空间换时间*/
float* load_fast_dis(Node*start, Node*last, float fast[500])
{
	int i = 1;
	fast[0] = 0;//待优化，可以由是否越过起始点和距离起始点距离来改进
	float temp_dis;
	while (start->prior != last)
	{
		temp_dis = distance(start->latitude, start->longitude, start->next->latitude, start->next->longitude);
		fast[i] = temp_dis + fast[i - 1];
		//printf("目前距离为：%f\n", fast[i]);
		i++;
		start = start->next;
	}
	//将最后所有数据填充为最后一个速度，方便之后实时计算时间
	int last_index = i - 1;
	for (i; i < 500; i++)
	{
		fast[i] = fast[last_index];
	}
	return fast;
}
int* load_fast_time(Node*start, Node*last, int time[500])
{
	int i = 0;

	while (start->prior != last)
	{
		time[i] = start->seconds;
		i++;
		start = start->next;
	}
	//将最后所有数据填充为enumber
	int last_index = i - 1;
	for (i; i < 500; i++)
	{
		time[i] = enumber;

	}
	return time;

}
void update_fast_lap(float lap_time, float &minlaptime, Node*last_start, Node*p)
{
	if (lap_time < minlaptime)
	{
		minlaptime = lap_time;
		load_fast_speed(last_start, p, fastest_lap_speed, fastest_lap_lat, fastest_lap_lng);//更新最快圈速度,经纬度
		load_fast_dis(last_start, p, fastest_lap_dis);//更新最快圈每段距离
		load_fast_time(last_start, p, fastest_lap_time);//更新最快圈每段时间戳
		fast_start = last_start;
		fast_end = p;
	}
}
//dis为当前车走过的总距离，start_time_lap是本圈起始时的时间，now_seconds是当前时间
float get_timediff(float dis, int start_time_lap, int now_seconds)
{
	float now_time = get_diff_time(now_seconds, start_time_lap);//当前圈当前用时
	if (lap_count == 1)
	{
		printf("当前为第一圈");
		return 0;
	}
	for (int i = 0; i < 499; i++)
	{

		if (fastest_lap_dis[i] <= dis && fastest_lap_dis[i + 1] > dis)
		{
			//最快圈在这个距离时  时间的计算方式有多种。
			//在最快圈每两点中寻找点，当前点里程在两点之间，算出最快圈在那的时间
			//最快圈的时间可能不可以由i得出
			float acceleration = (fastest_lap_speed[i + 1] - fastest_lap_speed[i]) / 0.1;//加速度
			float fast_node_time = get_diff_time(fastest_lap_time[i], fastest_lap_time[0]);//最快圈当前节点时间，还要加上动力学系统插值时间
			float fast_time = 0;//当前里程真正时间
			float diff_dis = dis - fastest_lap_dis[i];
			if (acceleration != 0)
				fast_time = (sqrt(fastest_lap_speed[i] * fastest_lap_speed[i] + 2 * acceleration*diff_dis) - fastest_lap_speed[i]) / acceleration * 3600 + fast_node_time;//动力学模型 公式
			else
				fast_time = diff_dis / fastest_lap_speed[i] * 3600 + fast_node_time;
			float timediff = fast_time - now_time;
			printf("当前时间%d,当前圈当前时间：%.2f,最快圈当前时间：%.2f,领先时间：%.2f\n", now_seconds, now_time, fast_time, timediff);
			return timediff;//单位为秒
		}
	}
	printf("里程未找到 里程为%f 时间为%d\n", dis, now_seconds);
	return 0;
}

float get_timediff1(float lat, float lng, int start_time_lap, int now_seconds, int &new_index,float last)
{
	/*lap 1*/
	if (lap_count == 1)
	{
		printf("当前为第一圈");
		return 0;
	}
	

	int close_index       = 0;                                                                           //寻找最快圈节点时使用的下标，计算新的一圈时需置零 
	bool flag_close_edge  = false;                                                                       //最近点序号,flag为1代表是离最快圈旧点更近
	float *now_time		  = (float *)malloc(sizeof(float));		                             //当前圈当前用时
	float *close_dis      = (float *)malloc(sizeof(float));                                              //三角中最近点距离
	float *dis            = (float *)malloc(sizeof(float));                                              //三角中另一点距离
	float *across_dis     = (float *)malloc(sizeof(float));                                              //垂线时差值距离
	float *acceleration   = (float *)malloc(sizeof(float));                                              //加速度
	float *fast_node_time = (float *)malloc(sizeof(float));                                              //最快圈到此节点时所用时间
	float *dis0           = (float *)malloc(sizeof(float));                                              //离当前节点最近的两个最快圈节点之间的距离
	float *fast_time      = (float *)malloc(sizeof(float));                                              //最快圈在当前节点所在位置的估计时间
	float timediff        = 0;
	*now_time             =get_diff_time(now_seconds, start_time_lap);
	*close_dis            = enumber;
	*dis                  = enumber;
	*across_dis           = 0;
	*acceleration         = 0;
	*fast_node_time       = 0;
	*dis0                 = 0;
	*fast_time            = 0;
	/*找到一个下标close_index*/
	for (int i = new_index; i < 499; i++)
	{
		float temp_dis = distance(fastest_lap_lat[i], fastest_lap_lng[i], lat, lng);
		/*搜索最近的两个点*/
		if (temp_dis < *close_dis)
		{
			*close_dis = temp_dis;
		
			bool end = fastest_lap_lat[i + 1] == enumber;
			float dis2 = distance(fastest_lap_lat[i + 1], fastest_lap_lng[i + 1], lat, lng);//后一个点的距离
			float dis3 = distance(fastest_lap_lat[i - 1], fastest_lap_lng[i - 1], lat, lng);//前一个
			if (end || dis2 > dis3)                                                         //离旧点更近，或到终点
			{
				close_index = i - 1;
				*dis = dis3;
				flag_close_edge = true;
			}
			else
			{
				close_index = i;
				*dis = dis2;
			}
			
		}
		flag_close_edge = false; 
		new_index = i;
	}
	/*搜索结束时close_dis是最小距离，dis是第二小距离，二者必相邻，close_index是前一个点下标*/
	*dis0 = distance(fastest_lap_lat[close_index], fastest_lap_lng[close_index], fastest_lap_lat[close_index + 1], fastest_lap_lng[close_index + 1]);
	if (*dis0 == 0)
		return real_segtime[real_segtime_index - 1];
	if (!flag_close_edge)
	{
		*across_dis = (pow(*dis, 2) + pow(*dis0, 2) - pow(*close_dis, 2)) / 2 / *dis0;                    //余弦函数计算插值距离,close在下
		if (*across_dis < 0)
		{
			printf("it do happen:%f",*dis);
			*across_dis = -*across_dis + *dis0;
		}
	}
	else
	{
		*across_dis = (pow(*close_dis, 2) + pow(*dis0, 2) - pow(*dis, 2)) / 2 / *dis0;				     //余弦函数计算插值距离，close在上
	}
	*acceleration = (fastest_lap_speed[close_index + 1] - fastest_lap_speed[close_index]) / 0.1;//加速度
	*fast_node_time = get_diff_time(fastest_lap_time[close_index], fastest_lap_time[0]);
	if (*fast_node_time == 0)
	{
		printf("fast_node_time=0 %d  %d", close_index, fastest_lap_time[0]);
		//return last;
	}


	if (*acceleration != 0)
		*fast_time = (sqrt(fastest_lap_speed[close_index] * fastest_lap_speed[close_index] + 2 * (*acceleration)*(*across_dis)) - fastest_lap_speed[close_index]) / (*acceleration) * 3600 + (*fast_node_time);//动力学模型 公式
	else
		*fast_time = *across_dis / fastest_lap_speed[close_index] * 3600 + *fast_node_time;
	timediff = *fast_time - *now_time;
	real_segtime[real_segtime_index] = *fast_time - *now_time;
	if (fastest_lap_lat[close_index + 2] == enumber)
		printf("meet the end of fastest_lap");
	
	
	/*滤波*/
	if (abs(timediff - last) >= 0.03)
	{
		printf("被迫滤波%f", last );
		if (timediff > last)
			timediff = last+0.01 ;
		else
			timediff = last-0.01 ;
	}
	/*写入文件*/

	new_index = close_index;
	real_segtime_index++;
	printf("当前时间%d,当前圈当前时间：%.2f,最快圈当前时间：%.2f,插值时间：%f,插值距离%f,领先时间：%.2f\n ",
		now_seconds, *now_time, *fast_node_time, *fast_time - *fast_node_time, *dis0, timediff);
	if (lap_count == 3)
		write_lap(timediff);
	/*释放申请的空间*/
	free(now_time);
	free(close_dis);
	free(across_dis);
	free(acceleration);
	free(fast_node_time);
	free(dis0);
	free(fast_time);
	free(dis);
	return timediff;//单位为秒
}
/*测试区代码，使用时需重写*/
void load_data(float direction[5000], float lat[5000], float lng[5000], int time[5000], float speed[5000])
{
	read_dire2node_once("C:\\Users\\Administrator\\Desktop\\data\\dire_long.txt", direction);
	read_lat2node_once("C:\\Users\\Administrator\\Desktop\\data\\lat_long.txt", lat);
	read_lng2node_once("C:\\Users\\Administrator\\Desktop\\data\\lng_long.txt", lng);
	read_time2node_once("C:\\Users\\Administrator\\Desktop\\data\\time.txt", time);
	read_speed2node_once("C:\\Users\\Administrator\\Desktop\\data\\speed.txt", speed);
}
/*测试区end*/
