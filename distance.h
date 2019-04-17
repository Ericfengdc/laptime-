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
/*
double mysin(double x) 
{
	return (x - 1 / 6 *pow(x,3) + 1 / 120* pow(x,5) - 1 / 5040* pow(x,7));
}
double mycos(double x)
{
	return(1 - 1/2*pow(x,2)+1/24*pow(x,4)-1/720*pow(x,6)+1/40320*pow(x,8));
}
*/
double mysin(double x) 
{
	float y = B * x + C * x * abs(x);
	y = P * (y * abs(y) - y) + y;
	return y;
}
double mycos(double x) 
{
	return mysin(x+PI/2);
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
 double distance(double lat1,double lng1,double lat2,double lng2)
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

 
