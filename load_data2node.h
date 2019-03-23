#include<stdio.h>
#include <stdlib.h>
#include "mynode_list.h"
#include<string.h>
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
void load_data(const char*dire_path,const char* lat_path,const char*lng_path,Node*p)
{
	read_dire2node(dire_path,p);
	read_lat2node(lat_path,p);
	read_lng2node(lng_path,p);
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
float* read_lat2node_once(const char*path,float lat[5000])
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
