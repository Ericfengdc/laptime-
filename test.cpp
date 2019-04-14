//😂😂😂😂😂（这表情也可以放这？？？）
#include "pch.h"
#include <windows.h>
#include <iostream>
#include <math.h>
#include<time.h>
#include "load_data2node.h"
#include "distance.h"
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
float fastest_dis[500];
int fastest_time[500];
int lap_count = 0;
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
		return true;//越过起始点
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
	if (!iscrossed(last_start)) //上一个节点是没越过的
	{
		delta_t1 = -delta_t1;
	}
	if (!iscrossed(this_start)) //本次节点未越过起点
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
void set_start(Node*lap,Node*new_circle) 
{
	start_point_lat = new_circle->latitude;//找到后更新起点位置数据（旧起点是平均值，没有准确时间）
	start_point_lng = new_circle->longitude;
	lap_count = 1;//当前为第一圈
}
void update_dis(float&prior_dis,float&dis,float&next_dis,Node*p) 
{
	prior_dis = dis;
	dis = next_dis;
	next_dis = distance(start_point_lat, start_point_lng, p->latitude, p->longitude);
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
void update_mileage(float&seg_mileage,float&mileage,Node *p) 
{
	seg_mileage = distance(p->latitude, p->longitude, p->next->latitude, p->next->longitude);
	//总里程
	mileage += seg_mileage;
}
//更新最快圈数据
float* load_fast_speed(Node*start,Node*last,float fast[500])
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
float* load_fast_dis(Node*start, Node*last, float fast[500]) 
{
	int i = 1;
	fast[0] = 0;//待优化，可以由是否越过起始点和距离起始点距离来改进
	float temp_dis;
	while (start->next != last)
	{
		temp_dis = distance(start->latitude,start->longitude,start->next->latitude,start->next->longitude);
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

	while (start->next != last)
	{
		time[i] = start->seconds;
		i++;
		start = start->next;
	}
	//将最后所有数据填充为最后一个速度，方便之后实时计算时间
	int last_index = i - 1;
	for (i; i < 500; i++)
	{
		time[i] = time[last_index];
		
	}
	return time;
	
}
//dis为当前车走过的总距离，start_time_lap是本圈起始时的时间，now_seconds是当前时间
float get_timediff(float dis,int start_time_lap,int now_seconds)
{
	float now_time = get_diff_time(now_seconds, start_time_lap);//当前圈当前用时
	if (lap_count == 1)
	{
		printf("当前为第一圈\n");
		return 0;
	}
	for (int i = 0; i < 500; i++) //i是当前序号也是当前圈时间
	{

		if (fastest_dis[i] <= dis && fastest_dis[i + 1] >  dis) 
		{
			//最快圈在这个距离时  时间的计算方式有多种。
			//在最快圈每两点中寻找点，当前点里程在两点之间，算出最快圈在那的时间
			//最快圈的时间可能不可以由i得出
			float acceleration= (fastest_speed[i + 1] - fastest_speed[i]) / 0.1;//加速度
			float fast_node_time = get_diff_time(fastest_time[i], fastest_time[0]);//最快圈当前节点时间，还要加上动力学系统插值时间
			float fast_time = 0;//当前里程真正时间
			float diff_dis = dis - fastest_dis[i];
			if (acceleration != 0)
				fast_time = (sqrt(fastest_speed[i] * fastest_speed[i] + 2 * acceleration*diff_dis) - fastest_speed[i]) / acceleration * 3600 +fast_node_time;//动力学模型 公式
			else
				fast_time = diff_dis / fastest_speed[i]*3600+ fast_node_time;
			float timediff =fast_time- now_time ;
			printf("当前时间%d,当前圈当前时间：%f,最快圈当前时间：%f,领先时间：%.2f\n",now_seconds,now_time,fast_time,timediff);
			return timediff;//单位为秒
		}
	}
	return 0;
}
/*数据丢弃，上一圈的数据丢弃*/
Node*delete_lap(Node*lap) 
{
	return NULL;
}
/*测试区代码，使用时需重写*/
void load_data(float direction[5000],float lat[5000],float lng[5000],int time[5000],float speed[5000])
{
	read_dire2node_once("C:\\Users\\Administrator\\Desktop\\data\\dire_long.txt", direction);
	read_lat2node_once("C:\\Users\\Administrator\\Desktop\\data\\lat_long.txt", lat);
	read_lng2node_once("C:\\Users\\Administrator\\Desktop\\data\\lng_long.txt", lng);
	read_time2node_once("C:\\Users\\Administrator\\Desktop\\data\\time.txt", time);
	read_speed2node_once("C:\\Users\\Administrator\\Desktop\\data\\speed.txt", speed);
}
/*测试区end*/
int main()
{
	float direction[5000];
	float lat[5000];
	float lng[5000];
	int time[5000];
	float speed[5000];
	int index=0,fast_index=0,start_time_lap = 0;//节点索引，最快圈索引,本圈起始时间
	float dis = 0, next_dis = 0, prior_dis = 0, minlaptime = enumber;
	//当前圈里程和当前圈当前段里程
	float mileage = 0, seg_mileage = 0;
	//新建节点为整个过程起点
	Node*lap=create_list(1);
	Node*new_circle = NULL;//保存第一圈起点
	Node*p = NULL;//计算时间时指向当前节点
	Node*last_start = NULL;//指向上一次的起点

	//加载模拟数据
	load_data(direction, lat, lng, time, speed);
	while (index<4724)
	{
		//每次进入先新建节点
		Node*new_node = create_node();
		//Sleep(100);
		fill_node_once(direction[index], lat[index], lng[index], time[index], speed[index], new_node);//调用小尘接口（放在参数里）
		insert_list(lap, new_node);
		//当50秒之后开始找起点
		if(len_list(lap)>=time_find_start&&start_point_lat==enumber)
		{
			get_straight_dire(lap);//找到直道数据
			new_circle = get_start_point(lap);//找到后面圈的起点
			set_start(lap,new_circle);//设置起点信息，当前圈数
			start_time_lap = new_circle->seconds;//当前圈起始时间
		}
		if(new_circle)
		{	
			/*首次初始化,完成后在new_circle和第500个节点之间试图寻找一圈终点*/
			if (!p)
			{
				/*删除前面的无用节点*/
				while (lap->next != new_circle)
				{
					lap = delect_start_list(lap);
					printf("长度：%d\n", len_list(lap));
				}
				p = new_circle->next; 
				//记录上一圈的起点节点，用来计算下一圈的时间
				last_start = new_circle;
				/*在new_circle和第500个节点之间试图寻找一圈终点*/
				while (p->next != NULL)
				{
					//上段距离，这里计算上段距离有原因，因为下面p = new_node->prior之后调整过来计算当前段了
					update_mileage(seg_mileage,mileage,p->prior);
					//dis们的更新
					update_dis(prior_dis,dis,next_dis,p->next);
					//距离限度
					if (dis < dis_limit&&dis < next_dis&&dis < prior_dis) 
					{
						printf("距离为：%f  时间是：%d 速度是：%f\n", dis, p->seconds, p->speed);
						printf("本圈时间为：%.2f\n", laptime(last_start, p));//计算此圈圈速
						printf("本圈距离：%.3f km\n", mileage);
						last_start = p;//上一个起点的更新
						start_time_lap = p->seconds;//更新起始时间
					}
					p = p->next;	
					fast_index++;
				}
				/*试图寻找结束*/
			}
			
			//在这之前从上一个if出来p==new_node,需要调整
			p = new_node->prior;//p指向最新节点的前一个，因为需要与后一个节点判断距离差（assert p->next!=NULL）,一开始p是第500个node
			//新节点更新里程
			update_mileage(seg_mileage,mileage,p);
			//dis们的更新,寻找离起点最近的点
			update_dis(prior_dis, dis, next_dis, new_node);
			//打印当前点相对最快圈时间差距
			get_timediff(mileage, start_time_lap, p->seconds);

			if (dis < dis_limit&&dis < next_dis&&dis < prior_dis) //距离限度
			{
				float lap_time = laptime(last_start, p);
				printf("领先时间%f\n", minlaptime - lap_time);
				if (lap_time < minlaptime)
				{
					minlaptime = lap_time;
					load_fast_speed(last_start, p, fastest_speed);//更新最快圈速度
					load_fast_dis(last_start, p, fastest_dis);//更新最快圈每段距离
					load_fast_time(last_start,p,fastest_time);//更新最快圈每段时间戳
				}
				
				printf("距离为：%f  时间是：%d 速度是：%f\n", dis, p->seconds, p->speed);
				printf("本圈时间为：%.2f\n", lap_time);//打印此圈时间
				printf("本圈距离：%.3f km\n", mileage);
				/*删除上一圈的节点*/
				while (lap->next != p) 
				{
					lap = delect_start_list(lap);
					printf("长度%d\n",len_list(lap));
				}
				//新一圈更新的数据
				last_start = p;//上一个起点的更新
				start_time_lap = p->seconds;
				mileage = 0;//总里程清零
				lap_count++;
			}
			p = p->next;																										
			//开始计算实时时间差
		}
		index++;
	}
}
