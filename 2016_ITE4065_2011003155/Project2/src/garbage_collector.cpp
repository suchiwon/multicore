/*
@FILE:		/src/gabage_collector.cpp
@DATE:		2016/10/29
@AUTHOR:	서치원(suchiwon@hanyang.ac.kr)
@BRIEF:		각 스레드의 버전 리스트의 메모리 오버플로우 방지용 가비지 콜렉터
*/
#include "../include/global.h"

/**
스레드의 실행 함수
인자:
	arg: main함수에서 넘어온 인자. 가비지 콜렉터 스레드 번호.
*/
void* ThreadFuncGarbageCollector(void* arg) {
    
    int b_collect_min_version = (*(int*)arg);

    unsigned long version_temp;

    //메인 함수 타이머 루프 종료시 까지 실행
    while (!g_duration_end) {

	//가비지 콜렉터 0번이 활성화 리스트 중 가장 오래된 버전을 탐색했으나
	//현재 이 역할은 각 계산 스레드가 실행함
        /*
        if (b_collect_min_version == 0) {

            pthread_mutex_lock(&g_mutex);

                version_temp = FindOldestActiveVersion(g_active_thread_list_head);

            pthread_mutex_unlock(&g_mutex);

            if (g_min_active_version < version_temp) {
                g_min_active_version = version_temp;
            }
        }
        */

    	//각 스레드를 돌면서 더 이상 읽지 않는 버전들 삭제
        for (int i = 0; i < g_thread_num; i++) {
            DeleteUnreadVersion(i, g_min_active_version);    
        }
    }

    return NULL;
}
