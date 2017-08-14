/*
@FILE:		/include/global.h
@DATE:		2016/10/02
@AUTHOR:	서치원(suchiwon@hanyang.ac.kr)
@BRIEF:		cpp 파일에서 사용되는 전역 변수들과 초기화 함수,
			최대, 최소값 계산 함수 선언

*/

#ifndef __PROJECT1_SRC_GLOBAL_H__
#define __PROJECT1_SRC_GLOBAL_H__

#include <cstdio>
#include <cstring>
#include <vector>
#include <pthread.h>
using namespace std;

/**
전역 상수 선언부
*/
const int INF = 1000000000;			//집까지의 거리 중 max 변수 초기화 값
const int MAX_CORE_NUM = 36;		//서버 환경의 코어 갯수  
const int MAX_THREAD_NUM = 22;		//동시에 진행할 최대 bfs 수
const int MAX_HOUSE_NUM = 64;		//선언되는 집의 최대 갯수
const int MAX_EDGE_SIZE = 10;		//한 교차로가 가질 수 있는 최대 거리 갯수
/**
전역 변수 선언부
*/
int g_house_num;					//선언된 집의 갯수(MAX:64)
int g_vertex_size;					//그래프의 교차로 갯수(MAX:INT_MAX)
int g_max;							//집 사이의 최소 거리 중 최대값
int g_min;							//집 사이의 최소 거리 중 최소값

/**
전역 변수 선언부 - 그래프와 탐색 정보 저장용 변수들

배열의 사이즈는 그래프의 교차로 개수 g_vertex_size 변수에 의해 열의 개수가 결정

g_house_vertex_num 배열에는 각 집의 번호가 읽은 순서대로 저장

ex) g_house_vertex_num[0]	= 10  -> 첫 번째 집의 교차로 번호가 10
*/
vector< vector<int> > g_graph;		            			//그래프 저장용 가변 배열
vector<short> g_edge_size;					                //각 교차로의 거리 갯수 저장 배열
int g_house_vertex_num[MAX_HOUSE_NUM];                      //집의 교차로 번호와 bfs 탐색 배열의 인덱스를 맞추기 위한 저장 배열

vector< vector<char> > g_is_visited;						//교차로를 방문했는지의 여부 저장 배열
vector< vector<int> >  g_depth_from_house;					//출발지부터의 각 교차로의 거리 저장 배열
vector< vector<int> >  g_vertex_queue;						//bfs 탐색을 위한 현재 depth 교차로 저장 큐
vector< vector<int> >  g_vertex_queue_copy;					//다음 depth 교차로 저장 큐
vector<int> g_visited_num;									//현재 탐색한 교차로의 수 배열

char g_thread_finish[MAX_THREAD_NUM];						//스레드의 탐색 종료 여부 배열
int g_execute_bfs_num;										//현재 실행중인 스레드의 갯수
int g_parallel_bfs;											//현재 병렬 bfs로 실행중인 스레드 번호				
	
pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;		//bfs 탐색시 각 스레드의 mutex_lock을 위한 변수
pthread_mutex_t g_mutex_bfs = PTHREAD_MUTEX_INITIALIZER;	//병렬 bfs에서의 mutex_lock을 위한 변수

void Init();							                    //전역 변수 초기화 함수. main 함수에서 사용

#endif // __PROJECT1_SRC_GLOBAL_H__
