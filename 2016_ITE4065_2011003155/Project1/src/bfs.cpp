/*
@FILE:		/src/bfs.cpp
@DATE:		2016/10/02
@AUTHOR:	��ġ��(suchiwon@hanyang.ac.kr)
@BRIEF:		�������� ���� �� ���� �Լ��� bfs Ž�� �Լ� ����
*/

#include "../include/global.h"
#include <string.h>
/**
���� bfs�� ���� ������ ���� �Լ��� ���� �������� ����ü
*/
typedef struct _MULTI_BFS_ARGS {
    int thread_idx;					//���� ������ ��ȣ
    int start_idx;					//���� depth ť���� Ž���� ������ ����(����:0 ~ jump_size - 1)
    int depth;						//���� Ž�� ���� �Ÿ�
    int jump_size;					//Ž���� �ǳʶ� ũ��(=���� �������� ����)
} MULTI_BFS_ARGS;

//���� bfs ���� �Լ� ����
void *ThreadFuncInBfs(void* arg);
/**
�� �������� �Ÿ��� ����� �� �ִ�,�ּҰŸ��� ���ϴ� bfs Ž�� �Լ�
����:
	thread_idx:		  �������� ��ȣ(0 ~ MAX_THREAD_NUM - 1)
	start_vertex_num: bfs Ž���� ���� ���� ������ ��ȣ
*/
void Bfs(const int thread_idx, const int start_vertex_num) {

	//���� bfs�� ���� ������ ��ȣ ���� �迭�� �Ѱ��� ����ü�� �迭
    pthread_t threads[MAX_CORE_NUM];
    MULTI_BFS_ARGS multi_bfs_args[MAX_CORE_NUM];

	//���� bfs ���� ����ü�� �ʱ�ȭ
    for (int i = 0; i < MAX_CORE_NUM; i++) {
        multi_bfs_args[i].thread_idx = thread_idx;
        multi_bfs_args[i].start_idx = i;
    }

    //bfs Ž���� ����� �ӽ� ���� ����
    int cur_idx, depth, adj_idx, thread_size;

    //Ž�� �� �ʱ�ȭ
    //ť�� ��� ������ �߰��ϰ� �ڽŰ��� �Ÿ��� 0���� �ʱ�ȭ, �׸��� ��Ž�� ������ ���� �湮 ���� üũ
    g_is_visited[thread_idx].assign(g_vertex_size + 1, 0);
    
    g_vertex_queue_copy[thread_idx].clear();

    g_vertex_queue_copy[thread_idx].push_back(start_vertex_num);
    g_depth_from_house[thread_idx][start_vertex_num] = 0;
    g_is_visited[thread_idx][start_vertex_num] = 1;
    g_visited_num[thread_idx] = 1;
    depth = 0;

    //ť�� ��������(=��������� ������ �� �ִ� ��� �������� Ž���� ����������) Ž��
    while (g_visited_num[thread_idx] < g_vertex_size) {
        
		//���� Ž�� �Ÿ� ����
        ++depth;

		//���� depth���� Ž���ߴ� �����θ� ���� Ž�� ť�� ����
        g_vertex_queue[thread_idx].assign(g_vertex_queue_copy[thread_idx].begin(), g_vertex_queue_copy[thread_idx].end());

		//���� Ž���� ť�� ���� �� ����
        if (g_vertex_queue[thread_idx].size() == 0) {
            break;
        }

		//���� depth���� Ž���� �������� ������ ���� �ʱ�ȭ
        g_vertex_queue_copy[thread_idx].clear();
       

		//���� �ڽ��� ���� bfs ���� �������� ��� ���� bfs ����
        if (g_parallel_bfs == thread_idx && g_vertex_queue[thread_idx].size() > 1000) {

			//�������� ���� ���� �ؼҸ� ���� ������ ���� ����
            thread_size = (MAX_CORE_NUM - g_execute_bfs_num - 2);

			//�����带 ������ ���� depth���� ť�� ���ķ� Ž��
            for (int i = 0; i < thread_size; i++) {
                multi_bfs_args[i].jump_size = thread_size;
                multi_bfs_args[i].depth = depth;

                if (pthread_create(&threads[i], 0, ThreadFuncInBfs, &multi_bfs_args[i]) < 0) {
                    printf("pthread_create error in bfs!!\n");
                    return;
                }
            }

            for (int i = 0; i < thread_size; i++) {
                 pthread_join(threads[i], NULL);
            }

			//Ž���� ������ ������ Ž�� �Ϸ� ������ ������ ����
            g_visited_num[thread_idx] += g_vertex_queue_copy[thread_idx].size();

		//���� bfs ��� �����尡 �ƴ� ��� �Ϲ� bfs ����
        } else {

            for (int i = 0; i < g_vertex_queue[thread_idx].size(); i++) {
            
                cur_idx = g_vertex_queue[thread_idx][i];

                for (int j = 0; j < g_edge_size[cur_idx]; j++) {
                
                    adj_idx = g_graph[j][cur_idx];

                    if (g_is_visited[thread_idx][adj_idx] == 0) {
                        g_is_visited[thread_idx][adj_idx] = 1;
                        g_depth_from_house[thread_idx][adj_idx] = depth;
                        g_vertex_queue_copy[thread_idx].push_back(adj_idx);
                    }
                }
            }

			//Ž���� ������ ������ Ž�� �Ϸ� ������ ������ ����
            g_visited_num[thread_idx] += g_vertex_queue_copy[thread_idx].size();
        }
    }

    //��� Ž���� ����� �� ���� �� ������ �Ÿ��� ���� �ִ�, �ּҰ� ���
    //���� ������ �� �����尡 ���� ���� �� �� �����Ƿ� mutex_lock���� ���´�.
    pthread_mutex_lock(&g_mutex);
    for (int i = 0; i < g_house_num; i++) {
        depth = g_depth_from_house[thread_idx][g_house_vertex_num[i]];

        if (g_max < depth) {
            g_max = depth;
        }

        if (g_min > depth && depth > 0) {
            g_min = depth;
        }
    }
	
	//�ڽ��� ������ ���� �˸�
    g_thread_finish[thread_idx] = 1;

	//�ڽ��� ���� bfs ����̾��� ��� �ٸ� �����忡�� �ѱ�
    if (g_parallel_bfs == thread_idx) {
        for (int i = 0; i < MAX_THREAD_NUM; i++) {
            if (g_thread_finish[i] == 0) {
                g_parallel_bfs = i;
            }
        }
    }

	//���� ������ ���� ���� ����
    --g_execute_bfs_num;

    pthread_mutex_unlock(&g_mutex);
}

/**
�� �������� ������ ȣ��� �Լ�
���������� �� �������� �Ÿ��� ����ϴ� bfs �Լ� ����
����:
	arg: pthread_create���� �Ѿ�� ����. �Ÿ� ����� ���� ������ ��ȣ
*/
void *ThreadFunc(void *arg) {
    pair<int, int> arg_copy = *((pair<int, int>*)arg);

    Bfs(arg_copy.first, arg_copy.second);

    return NULL;
}
/**
���� bfs ���� �Լ�
bfs�� ������ Ž�� ť�� ���ķ� Ž��
����:
	arg: MULTI_BFS_ARGS ���� bfs Ž���� �ʿ��� ���� ���� ����ü
*/
void *ThreadFuncInBfs(void *arg) {

    int cur_idx, adj_idx;

    MULTI_BFS_ARGS arg_copy = *((MULTI_BFS_ARGS*)arg);

	//�ڽ��� �������� Ž���� ������ �ӽ� ���� �迭
    vector<int> vertex_copy;

	//�ڽ��� Ž���� ���� �������� ���� bfs ������ ũ�� ��ŭ �ǳʶٸ鼭 Ž��
    for (int i = arg_copy.start_idx; i < g_vertex_queue[arg_copy.thread_idx].size(); i += arg_copy.jump_size) {
            
        cur_idx = g_vertex_queue[arg_copy.thread_idx][i];
            
        for (int j = 0; j < g_edge_size[cur_idx]; j++) {
               
            adj_idx = g_graph[j][cur_idx];

			/**
			�� �κп��� Ž�� ������ �˻��� �浹�� ���� �����尡 ���� �����θ� Ž���� ���ɼ��� ������
			���信 ������ ��ġ�� �ʰ� lock�� ����� �����ϴ� ���� �ʹ� ũ�Ƿ� lock�� ������� ����
			*/
            if (g_is_visited[arg_copy.thread_idx][adj_idx] == 0) {
                g_is_visited[arg_copy.thread_idx][adj_idx] = 1;
                g_depth_from_house[arg_copy.thread_idx][adj_idx] = arg_copy.depth;
		        vertex_copy.push_back(adj_idx);
            }
        }
    }

	//���� Ž�� ť�� �����ϴ� ������ �浹 �� Ž���ؾ��� ������ ������ ����� �� �����Ƿ� lock�� ��
    pthread_mutex_lock(&g_mutex_bfs);
    g_vertex_queue_copy[arg_copy.thread_idx].insert(g_vertex_queue_copy[arg_copy.thread_idx].end(),vertex_copy.begin(),vertex_copy.end());
    pthread_mutex_unlock(&g_mutex_bfs);
}

/**
�����带 ������ ���ÿ� bfs�� �����ϰ� Ž�� �Ϸ� �� �ִ�, �ּҰ��� ����� �����ϴ� �Լ�

�޸��� �������� �� ���� �����ϴ�(=�����ϴ�) ������� �ִ� ������ ����(22) ��ŭ ���� ��
������ �ھ ���� bfs�� ���.
�̷��� �������� ���� �����带 ������ ��� �������� �Ÿ��� Ž���� ������ ����
*/
void MakeThreadsAndSolve() {

    //������ ID ���� �迭�� bfs �迭�� �� ���ڸ� �Ѱ��ֱ� ���� �迭
    pthread_t threads[g_house_num];

    int not_solve_house_num = g_house_num;		//���� Ž���ؾ��� ���� ����
    int iter_num = 0;							//������ ��ȣ�� ���� ������ ��ȣ�� ���߱� ���� ����
    int house_vertex;							//Ž���� ���� ������ ��ȣ �ӽ� ���� ����
    int execute_bfs_num_saver;					//���ÿ� Ž���� ������ bfs�� ���� ���� ����

    pair<int, int> idx_saver[MAX_THREAD_NUM];	//�����忡 �Ѱ��� ���� ���� �迭
												//first:������ ��ȣ  /second:���� ������ ��ȣ

    //��� ���� Ž���� ���������� �ݺ�
    while (not_solve_house_num > 0) {

		//���� ���� �� ���� ������ ������ ���� �ϰ�� ���� �� ��ŭ�� ����
        if (not_solve_house_num < MAX_THREAD_NUM) {
            g_execute_bfs_num = not_solve_house_num;
        } else {
            g_execute_bfs_num = MAX_THREAD_NUM;
        }

		//���� ������ ���� ���� ���� ������ ������ ����ǹǷ� ������� ������ ���� ����
        execute_bfs_num_saver = g_execute_bfs_num;

		//�������� �Ϸ� ���¸� ���� false(0)���� ����
	    memset(g_thread_finish,0,sizeof(g_thread_finish));

		//ó�� ���� bfs�� ������ �������� ��ȣ ����
	    g_parallel_bfs = 0;

	    //�����带 ������ bfs�� ����. ��� �����尡 ����ɶ����� ���
        for (int i = 0; i < execute_bfs_num_saver; i++) {
            
			//���� ������ ��ȣ�� ���� ������ Ž���ؾ��� ���� ������ ��ȣ ����
            house_vertex = g_house_vertex_num[(iter_num * MAX_THREAD_NUM) + i];

            idx_saver[i] = make_pair(i,house_vertex);

            if (pthread_create(&threads[i], 0, ThreadFunc, &idx_saver[i]) < 0) {
                printf("pthread_create error!\n");
                return;
            }
        }

        for (int i = 0; i < execute_bfs_num_saver; i++) {
            pthread_join(threads[i], NULL);
        }

		//������ ���� Ƚ���� ����ϰ� Ž���� ���� ���� ó��
        ++iter_num;
        not_solve_house_num -= execute_bfs_num_saver;

    }
    
    //��� �������� ����� ����Ǹ� �ִ�, �ּҰ� ���
    printf("%d\n%d\n",g_min,g_max);

    return;
}
