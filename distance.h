#include <stdio.h>
#include <math.h>
#define PI                      3.1415926
#define EARTH_RADIUS            6378.137        //地球近似半径

double radian(double d);
double get_distance(double lat1, double lng1, double lat2, double lng2);

// 求弧度
double radian(double d)
{
	return d * PI / 180.0;   //角度1˚ = π / 180
}

//计算距离
double get_distance(double lat1, double lng1, double lat2, double lng2)
{
	double radLat1 = radian(lat1);
	double radLat2 = radian(lat2);
	double a = radLat1 - radLat2;
	double b = radian(lng1) - radian(lng2);

	double dst = 2 * asin((sqrt(pow(sin(a / 2), 2) + cos(radLat1) * cos(radLat2) * pow(sin(b / 2), 2))));

	dst = dst * EARTH_RADIUS;
	dst = round(dst * 10000) / 10000;
	return dst;
}
 double distance(float lat1,float lng1,float lat2,float lng2)
{

	//double lat1 = 39.90744;
	//double lng1 = 116.41615;//经度,纬度1
	//double lat2 = 39.90744;
	//double lng2 = 116.30746;//经度,纬度2
	// insert code here...

	double dst = get_distance(lat1, lng1, lat2, lng2);
	//printf("dst = %0.3fkm\n", dst);  //dst = 9.281km
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
 //求数组方差
 float variance(float a[], int n)
 {
	 float sum = 0;
	 float average = 0;
	 for (int i = 0; i < n; i++)
	 {
		 sum += a[i];
	 }
	 average = sum / n;
	 sum = 0;
	 for (int i = 0; i < n; i++)
		 sum += (a[i] - average)*(a[i] - average);
	 return sum / n;
 }
 
