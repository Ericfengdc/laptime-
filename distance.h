#pragma once
#include <stdio.h>
#include <math.h>
#define PI                      3.1415926
#define EARTH_RADIUS            6378.137        //地球近似半径

double radian(double d);
double get_distance(double lat1, double lng1, double lat2, double lng2);

const float B = 4 / PI;
const float C = -4 / (PI*PI);
const float P = 0.225;
// 求弧度
double radian(double d)
{
	return d * PI / 180.0;   //角度1˚ = π / 180
}

double mysin(double x);
double mycos(double x);
//计算距离
double get_distance(double lat1, double lng1, double lat2, double lng2);
double distance(double lat1, double lng1, double lat2, double lng2);
float get_diff_time(int a, int b); //a大b小

/*计算时间等的函数*/
//找到最长直道的方向角等数据
void get_straight_dire(Node* head);

//找到最长直道信息后，用此函数找出数据中符合方向角，经纬度要求的起点节点，后续可能要用此节点的时间等信息
Node* get_start_point(Node *head);

//判断当前起点是否越过了真正起点
bool iscrossed(Node *p);

//计算圈时,公式为（t2-delta_t2）-(t1-delata_t1),t1为上圈时间，delta_t1为上一圈插值
float laptime(Node *last_start, Node* this_start);

//判断当前节点是不是离起点距离最短的
//abandoned(三次计算distance吃不消)
void set_start(Node*lap, Node*new_circle);

void update_dis(float&prior_dis, float&dis, float&next_dis, Node*p);

float sum_dis(Node*a, Node*b);

void update_mileage(float&seg_mileage, float&mileage, Node *p);

//更新最快圈数据，包含加载数据，距离，时间，已经整体更新
float* load_fast_speed(Node*start, Node*last, float fast[500], float lat[500], float lng[500]);


/*最快圈时储存每两个节点之间的距离，后期如果优化计算负载可以在此函数内不用distance计算*/
/*因为在计算总里程的时候更新过,但需要额外的空间来保存dis变量，以空间换时间*/
float* load_fast_dis(Node*start, Node*last, float fast[500]);

int* load_fast_time(Node*start, Node*last, int time[500]);

void update_fast_lap(float lap_time, float &minlaptime, Node*last_start, Node*p);

//dis为当前车走过的总距离，start_time_lap是本圈起始时的时间，now_seconds是当前时间
float get_timediff(float dis, int start_time_lap, int now_seconds);


float get_timediff1(float lat, float lng, int start_time_lap, int now_seconds, int &new_index);
/*测试区代码，使用时需重写*/
void load_data(float direction[5000], float lat[5000], float lng[5000], int time[5000], float speed[5000]);

 
