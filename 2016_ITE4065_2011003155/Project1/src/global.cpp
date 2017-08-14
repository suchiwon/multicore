/*
@FILE:		/src/global.cpp
@DATE:		2016/10/02
@AUTHOR:	��ġ��(suchiwon@hanyang.ac.kr)
@BRIEF:		global.h ���� ����� �Լ��� ����
*/

#include "../include/global.h"

/**
���� ���� �ʱ�ȭ �Լ�
*/
void Init() {

    int house_num = g_house_num;				//�ھ� ���� �̻��� �Ҵ��� ���� ���� ���� ����

    if (g_house_num > MAX_THREAD_NUM) {
        house_num = MAX_THREAD_NUM;    
    }
    
    //�׷��� ���� �迭 ũ�� �Ҵ�. 
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

    //���� �迭�� ũ�� �Ҵ�. �� �Լ����� �ʱ�ȭ �� �� �迭�� ũ�� ��ȭ ����
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
    //��� ���� �ִ�, �ּҰ� ���� ���� �ʱ�ȭ
    g_max = 0;
    g_min = INF;
}
