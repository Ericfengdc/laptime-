#pragma once
#include "mynode_list.h"
#include "distance.h"
/*赛后分析*/

float sum_mileage(Node*lap1_start, Node*lap1_end)
{
	float sum = 0;
	while (lap1_start != lap1_end)
	{
		sum += distance(lap1_start->latitude, lap1_start->longitude, lap1_start->next->latitude, lap1_start->next->longitude);
		lap1_start = lap1_start->next;
	}
	return sum;
}
void compare_time(Node*lap1_start, Node*lap1_end, Node*lap2_start, Node*lap2_end)
{
	float time_log[500];
	float sum1 = sum_mileage(lap1_start, lap1_end);
	float sum2 = sum_mileage(lap2_start, lap2_end);
	float scale = sum1 / sum2;//里程差的补偿
	float time1 = laptime(lap1_start,lap1_end);
	float time2 = laptime(lap2_start,lap2_end);
	float temp_mileage_lap1 = 0;
	Node*q = lap1_start->next;
	int index = 0;
	
	FILE*f = fopen("C:\\Users\\Administrator\\Desktop\\2.txt", "a");
	fprintf(f, "new lap ");
	fclose(f);
	while (q != lap1_end) 
	{

		Node*p = lap2_start;
		float temp_mileage_lap2 = 0;
		float temp_time_lap1 = get_diff_time(q->seconds, lap1_start->seconds);
		temp_mileage_lap1 +=distance(q->prior->latitude, q->prior->longitude, q->latitude, q->longitude);
		while (p->next!= lap2_end) 
		{
			float next_mileage=distance(p->latitude, p->longitude, p->next->latitude, p->next->longitude);
			if (temp_mileage_lap2*scale < temp_mileage_lap1 && (temp_mileage_lap2 + next_mileage)*scale >= temp_mileage_lap1)
			{
				//找到lap2对应的里程为p,p->next之间
				float temp_node_time_lap2 = get_diff_time(p->seconds,lap2_start->seconds);
				float temp_time_lap2 = 0;
				float diff_mileage = temp_mileage_lap1 - temp_mileage_lap2*scale;
				float acceleration = (p->next->speed - p->speed) / 0.1;
				if (acceleration != 0)
					temp_time_lap2 = (sqrt(p->speed * p->speed + 2 * acceleration*diff_mileage) - p->speed) / acceleration * 3600 + temp_node_time_lap2;//动力学模型 公式
				else
					temp_time_lap2 = diff_mileage / p->speed* 3600 + temp_node_time_lap2;
				//printf("lap2：%f scale:%f\n", temp_time_lap1 -temp_time_lap2,scale);
				FILE*f = fopen("C:\\Users\\Administrator\\Desktop\\2.txt", "a");
				fprintf(f, "%.2f \n", temp_time_lap1 - temp_time_lap2);
				fclose(f);
				time_log[index] = temp_time_lap1 - temp_time_lap2;
				break;
			}
			temp_mileage_lap2 += next_mileage;
			p = p->next;
		}
		q = q->next;
		index++;
		
	}

	/*滤波，平滑图像
	time_log[0] = 0;
	for (int i = 1; i-5 < index; i+=5) 
	{
		time_log[i + 1] = time_log[i] + 0.2*(time_log[i + 5] - time_log[i]);
		time_log[i + 2] = time_log[i] + 0.4*(time_log[i + 5] - time_log[i]);
		time_log[i + 3] = time_log[i] + 0.6*(time_log[i + 5] - time_log[i]);
		time_log[i + 4] = time_log[i] + 0.8*(time_log[i + 5] - time_log[i]);
	}
	time_log[index - 1] = time1 - time2;
	time_log[index - 2] = time_log[index - 6] + 0.2*(time_log[index - 1] - time_log[index - 6]);
	time_log[index - 3] = time_log[index - 6] + 0.4*(time_log[index - 1] - time_log[index - 6]);
	time_log[index - 4] = time_log[index - 6] + 0.6*(time_log[index - 1] - time_log[index - 6]);
	time_log[index - 5] = time_log[index - 6] + 0.8*(time_log[index - 1] - time_log[index - 6]);
	for (int i = 0; i < index; i++)
	{
		printf("diff:%f\n", time_log[i]);
		FILE*f = fopen("C:\\Users\\Administrator\\Desktop\\2.txt", "a");
		fprintf(f, "%.2f \n", time_log[i]);
		fclose(f);
	}*/
}
