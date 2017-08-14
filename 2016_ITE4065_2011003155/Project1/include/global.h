/*
@FILE:		/include/global.h
@DATE:		2016/10/02
@AUTHOR:	��ġ��(suchiwon@hanyang.ac.kr)
@BRIEF:		cpp ���Ͽ��� ���Ǵ� ���� ������� �ʱ�ȭ �Լ�,
			�ִ�, �ּҰ� ��� �Լ� ����

*/

#ifndef __PROJECT1_SRC_GLOBAL_H__
#define __PROJECT1_SRC_GLOBAL_H__

#include <cstdio>
#include <cstring>
#include <vector>
#include <pthread.h>
using namespace std;

/**
���� ��� �����
*/
const int INF = 1000000000;			//�������� �Ÿ� �� max ���� �ʱ�ȭ ��
const int MAX_CORE_NUM = 36;		//���� ȯ���� �ھ� ����  
const int MAX_THREAD_NUM = 22;		//���ÿ� ������ �ִ� bfs ��
const int MAX_HOUSE_NUM = 64;		//����Ǵ� ���� �ִ� ����
const int MAX_EDGE_SIZE = 10;		//�� �����ΰ� ���� �� �ִ� �ִ� �Ÿ� ����
/**
���� ���� �����
*/
int g_house_num;					//����� ���� ����(MAX:64)
int g_vertex_size;					//�׷����� ������ ����(MAX:INT_MAX)
int g_max;							//�� ������ �ּ� �Ÿ� �� �ִ밪
int g_min;							//�� ������ �ּ� �Ÿ� �� �ּҰ�

/**
���� ���� ����� - �׷����� Ž�� ���� ����� ������

�迭�� ������� �׷����� ������ ���� g_vertex_size ������ ���� ���� ������ ����

g_house_vertex_num �迭���� �� ���� ��ȣ�� ���� ������� ����

ex) g_house_vertex_num[0]	= 10  -> ù ��° ���� ������ ��ȣ�� 10
*/
vector< vector<int> > g_graph;		            			//�׷��� ����� ���� �迭
vector<short> g_edge_size;					                //�� �������� �Ÿ� ���� ���� �迭
int g_house_vertex_num[MAX_HOUSE_NUM];                      //���� ������ ��ȣ�� bfs Ž�� �迭�� �ε����� ���߱� ���� ���� �迭

vector< vector<char> > g_is_visited;						//�����θ� �湮�ߴ����� ���� ���� �迭
vector< vector<int> >  g_depth_from_house;					//����������� �� �������� �Ÿ� ���� �迭
vector< vector<int> >  g_vertex_queue;						//bfs Ž���� ���� ���� depth ������ ���� ť
vector< vector<int> >  g_vertex_queue_copy;					//���� depth ������ ���� ť
vector<int> g_visited_num;									//���� Ž���� �������� �� �迭

char g_thread_finish[MAX_THREAD_NUM];						//�������� Ž�� ���� ���� �迭
int g_execute_bfs_num;										//���� �������� �������� ����
int g_parallel_bfs;											//���� ���� bfs�� �������� ������ ��ȣ				
	
pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;		//bfs Ž���� �� �������� mutex_lock�� ���� ����
pthread_mutex_t g_mutex_bfs = PTHREAD_MUTEX_INITIALIZER;	//���� bfs������ mutex_lock�� ���� ����

void Init();							                    //���� ���� �ʱ�ȭ �Լ�. main �Լ����� ���

#endif // __PROJECT1_SRC_GLOBAL_H__
