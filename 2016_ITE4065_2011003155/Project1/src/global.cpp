/*
@FILE:		/src/global.cpp
@DATE:		2016/10/02
@AUTHOR:	서치원(suchiwon@hanyang.ac.kr)
@BRIEF:		global.h 에서 선언된 함수의 정의
*/

#include "../include/global.h"

/**
전역 변수 초기화 함수
*/
void Init() {

    int house_num = g_house_num;				//코어 개수 이상의 할당을 막기 위한 지역 변수

    if (g_house_num > MAX_THREAD_NUM) {
        house_num = MAX_THREAD_NUM;    
    }
    
    //그래프 가변 배열 크기 할당. 
    g_graph.reserve(MAX_EDGE_SIZE);

    g_graph[0].reserve(g_vertex_size + 1);
    g_graph[1].reserve(g_vertex_size + 1);
    g_graph[2].reserve(g_vertex_size + 1);
    g_graph[3].reserve(g_vertex_size + 1);
    g_graph[4].reserve(g_vertex_size + 1);
    g_graph[5].reserve(g_vertex_size + 1);
    g_graph[6].reserve(g_vertex_size + 1);
    g_graph[7].reserve(g_vertex_size + 1);
    g_graph[8].reserve(g_vertex_size + 1);
    g_graph[9].reserve(g_vertex_size + 1);

    g_edge_size.resize(g_vertex_size + 1);

    //가변 배열의 크기 할당. 이 함수에서 초기화 된 후 배열의 크기 변화 없음
    g_is_visited.resize(house_num);
    g_depth_from_house.resize(house_num);
    g_vertex_queue.resize(house_num);
    g_vertex_queue_copy.resize(house_num);

    for (int i = 0; i < house_num; i++) {
        g_is_visited[i].reserve(g_vertex_size + 1);
        g_depth_from_house[i].reserve(g_vertex_size + 1);
        g_vertex_queue[i].reserve(g_vertex_size + 1);
        g_vertex_queue_copy[i].reserve(g_vertex_size + 1);
    }

    g_visited_num.reserve(house_num);
    //결과 값의 최대, 최소값 저장 변수 초기화
    g_max = 0;
    g_min = INF;
}
