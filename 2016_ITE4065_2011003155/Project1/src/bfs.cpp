/*
@FILE:		/src/bfs.cpp
@DATE:		2016/10/02
@AUTHOR:	서치원(suchiwon@hanyang.ac.kr)
@BRIEF:		스레드의 생성 및 관리 함수와 bfs 탐색 함수 정의
*/

#include "../include/global.h"
#include <string.h>
/**
병렬 bfs를 위해 스레드 실행 함수에 전할 변수들의 구조체
*/
typedef struct _MULTI_BFS_ARGS {
    int thread_idx;					//현재 스레드 번호
    int start_idx;					//현재 depth 큐에서 탐색을 시작할 지점(범위:0 ~ jump_size - 1)
    int depth;						//현재 탐색 중인 거리
    int jump_size;					//탐색을 건너뛸 크기(=병렬 스레드의 갯수)
} MULTI_BFS_ARGS;

//병렬 bfs 실행 함수 선언
void *ThreadFuncInBfs(void* arg);
/**
각 집에서의 거리를 계산한 후 최대,최소거리를 구하는 bfs 탐색 함수
인자:
	thread_idx:		  스레드의 번호(0 ~ MAX_THREAD_NUM - 1)
	start_vertex_num: bfs 탐색할 시작 집의 교차로 번호
*/
void Bfs(const int thread_idx, const int start_vertex_num) {

	//병렬 bfs를 위한 스레드 번호 저장 배열과 넘겨줄 구조체의 배열
    pthread_t threads[MAX_CORE_NUM];
    MULTI_BFS_ARGS multi_bfs_args[MAX_CORE_NUM];

	//병렬 bfs 인자 구조체의 초기화
    for (int i = 0; i < MAX_CORE_NUM; i++) {
        multi_bfs_args[i].thread_idx = thread_idx;
        multi_bfs_args[i].start_idx = i;
    }

    //bfs 탐색에 사용할 임시 저장 변수
    int cur_idx, depth, adj_idx, thread_size;

    //탐색 전 초기화
    //큐에 출발 지점을 추가하고 자신과의 거리를 0으로 초기화, 그리고 재탐색 방지를 위해 방문 여부 체크
    g_is_visited[thread_idx].assign(g_vertex_size + 1, 0);
    
    g_vertex_queue_copy[thread_idx].clear();

    g_vertex_queue_copy[thread_idx].push_back(start_vertex_num);
    g_depth_from_house[thread_idx][start_vertex_num] = 0;
    g_is_visited[thread_idx][start_vertex_num] = 1;
    g_visited_num[thread_idx] = 1;
    depth = 0;

    //큐가 빌때까지(=출발지에서 도달할 수 있는 모든 교차로의 탐색이 끝날때까지) 탐색
    while (g_visited_num[thread_idx] < g_vertex_size) {
        
		//현재 탐색 거리 증가
        ++depth;

		//이전 depth에서 탐색했던 교차로를 현재 탐색 큐로 복사
        g_vertex_queue[thread_idx].assign(g_vertex_queue_copy[thread_idx].begin(), g_vertex_queue_copy[thread_idx].end());

		//현재 탐색할 큐가 없을 시 종료
        if (g_vertex_queue[thread_idx].size() == 0) {
            break;
        }

		//현재 depth에서 탐색될 교차로의 저장을 위해 초기화
        g_vertex_queue_copy[thread_idx].clear();
       

		//현재 자신이 병렬 bfs 실행 스레드일 경우 병렬 bfs 실행
        if (g_parallel_bfs == thread_idx && g_vertex_queue[thread_idx].size() > 1000) {

			//스레드의 병목 현상 해소를 위해 스레드 갯수 제한
            thread_size = (MAX_CORE_NUM - g_execute_bfs_num - 2);

			//스레드를 생성해 현재 depth에서 큐를 병렬로 탐색
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

			//탐색한 교차로 갯수를 탐색 완료 교차로 갯수에 더함
            g_visited_num[thread_idx] += g_vertex_queue_copy[thread_idx].size();

		//병렬 bfs 대상 스레드가 아닐 경우 일반 bfs 실행
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

			//탐색한 교차로 갯수를 탐색 완료 교차로 갯수에 더함
            g_visited_num[thread_idx] += g_vertex_queue_copy[thread_idx].size();
        }
    }

    //모든 탐색이 종료된 후 계산된 집 사이의 거리를 통해 최대, 최소값 계산
    //전역 변수에 각 스레드가 동시 접근 할 수 있으므로 mutex_lock으로 막는다.
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
	
	//자신의 스레드 종료 알림
    g_thread_finish[thread_idx] = 1;

	//자신이 병렬 bfs 대상이었을 경우 다른 스레드에게 넘김
    if (g_parallel_bfs == thread_idx) {
        for (int i = 0; i < MAX_THREAD_NUM; i++) {
            if (g_thread_finish[i] == 0) {
                g_parallel_bfs = i;
            }
        }
    }

	//현재 스레드 실행 갯수 감소
    --g_execute_bfs_num;

    pthread_mutex_unlock(&g_mutex);
}

/**
각 스레드의 생성시 호출될 함수
집에서부터 각 교차로의 거리를 계산하는 bfs 함수 실행
인자:
	arg: pthread_create에서 넘어온 인자. 거리 계산할 집의 교차로 번호
*/
void *ThreadFunc(void *arg) {
    pair<int, int> arg_copy = *((pair<int, int>*)arg);

    Bfs(arg_copy.first, arg_copy.second);

    return NULL;
}
/**
병렬 bfs 실행 함수
bfs와 같으나 탐색 큐를 병렬로 탐색
인자:
	arg: MULTI_BFS_ARGS 형의 bfs 탐색에 필요한 정보 저장 구조체
*/
void *ThreadFuncInBfs(void *arg) {

    int cur_idx, adj_idx;

    MULTI_BFS_ARGS arg_copy = *((MULTI_BFS_ARGS*)arg);

	//자신의 영역에서 탐색한 교차로 임시 저장 배열
    vector<int> vertex_copy;

	//자신이 탐색할 시작 지점부터 병렬 bfs 스레드 크기 만큼 건너뛰면서 탐색
    for (int i = arg_copy.start_idx; i < g_vertex_queue[arg_copy.thread_idx].size(); i += arg_copy.jump_size) {
            
        cur_idx = g_vertex_queue[arg_copy.thread_idx][i];
            
        for (int j = 0; j < g_edge_size[cur_idx]; j++) {
               
            adj_idx = g_graph[j][cur_idx];

			/**
			이 부분에서 탐색 여부의 검사의 충돌로 여러 스레드가 같은 교차로를 탐색할 가능성이 있으나
			정답에 영향을 끼치지 않고 lock의 비용을 감수하는 것이 너무 크므로 lock를 사용하지 않음
			*/
            if (g_is_visited[arg_copy.thread_idx][adj_idx] == 0) {
                g_is_visited[arg_copy.thread_idx][adj_idx] = 1;
                g_depth_from_house[arg_copy.thread_idx][adj_idx] = arg_copy.depth;
		        vertex_copy.push_back(adj_idx);
            }
        }
    }

	//다음 탐색 큐에 저장하는 구간은 충돌 시 탐색해야할 교차로 정보가 사라질 수 있으므로 lock을 검
    pthread_mutex_lock(&g_mutex_bfs);
    g_vertex_queue_copy[arg_copy.thread_idx].insert(g_vertex_queue_copy[arg_copy.thread_idx].end(),vertex_copy.begin(),vertex_copy.end());
    pthread_mutex_unlock(&g_mutex_bfs);
}

/**
스레드를 생성해 동시에 bfs를 실행하고 탐색 완료 후 최대, 최소값의 계산을 실행하는 함수

메모리의 제한으로 한 번에 실행하는(=생성하는) 스레드는 최대 스레드 갯수(22) 만큼 생성 후
나머지 코어를 병렬 bfs에 사용.
이렇게 여러번에 걸쳐 스레드를 생성해 모든 집에서의 거리를 탐색할 때까지 실행
*/
void MakeThreadsAndSolve() {

    //스레드 ID 저장 배열과 bfs 배열의 행 인자를 넘겨주기 위한 배열
    pthread_t threads[g_house_num];

    int not_solve_house_num = g_house_num;		//현재 탐색해야할 집의 갯수
    int iter_num = 0;							//스레드 번호와 집의 교차로 번호를 맞추기 위한 변수
    int house_vertex;							//탐색할 집의 교차로 번호 임시 저장 변수
    int execute_bfs_num_saver;					//동시에 탐색을 시작한 bfs의 갯수 저장 변수

    pair<int, int> idx_saver[MAX_THREAD_NUM];	//스레드에 넘겨줄 인자 저장 배열
												//first:스레드 번호  /second:집의 교차로 번호

    //모든 집의 탐색이 끝날때까지 반복
    while (not_solve_house_num > 0) {

		//남은 집이 한 번에 생성할 스레드 이하 일경우 남은 집 만큼만 생성
        if (not_solve_house_num < MAX_THREAD_NUM) {
            g_execute_bfs_num = not_solve_house_num;
        } else {
            g_execute_bfs_num = MAX_THREAD_NUM;
        }

		//전역 변수인 현재 실행 중인 스레드 갯수가 변경되므로 실행시의 스레드 갯수 저장
        execute_bfs_num_saver = g_execute_bfs_num;

		//스레드의 완료 상태를 전부 false(0)으로 저장
	    memset(g_thread_finish,0,sizeof(g_thread_finish));

		//처음 병렬 bfs로 실행할 스레드의 번호 지정
	    g_parallel_bfs = 0;

	    //스레드를 생성해 bfs를 실행. 모든 스레드가 종료될때까지 대기
        for (int i = 0; i < execute_bfs_num_saver; i++) {
            
			//현재 스레드 번호와 보정 값으로 탐색해야할 집의 교차로 번호 구함
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

		//스레드 동시 횟수를 계산하고 탐색한 집의 갯수 처리
        ++iter_num;
        not_solve_house_num -= execute_bfs_num_saver;

    }
    
    //모든 스레드의 계산이 종료되면 최대, 최소값 출력
    printf("%d\n%d\n",g_min,g_max);

    return;
}
