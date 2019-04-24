#pragma once
#include<stdio.h>
#include <stdlib.h>
#include "mynode_list.h"
#include<string.h>
float* read_dire2node(const char *path, Node *head);
float* read_lat2node(const char *path, Node *head);
float* read_lng2node(const char *path, Node *head);
void load_data(const char*dire_path, const char* lat_path, const char*lng_path, Node*p);
float* read_dire2node_once(const char*path, float dire[5000]);
float* read_lat2node_once(const char*path, float lat[5000]);
float* read_lng2node_once(const char*path, float lng[5000]);
int* read_time2node_once(const char*path, int time[5000]);
void read_speed2node_once(const char*path, float speed[5000]);
