/*
@FILE:		/src/filter_lock.cpp
@DATE:		2016/10/29
@AUTHOR:	서치원(suchiwon@hanyang.ac.kr)
@BRIEF:		각 스레드의 활성화 리스트 변경의 충돌 방지용 락
*/
#include "../include/global.h"
#include <unistd.h>

vector<int> g_filter_level;		//각 단계의 대기실 배열
vector<int> g_filter_victim;		//각 단계의 양보 스레드 지정 배열

/**
락 변수 초기화
인자: 
	thread_num: 스레드의 갯수
*/
void FilterLockInit(const int thread_num) {
    g_filter_level.resize(thread_num);
    g_filter_victim.resize(thread_num);
}

/**
락 설정
인자: 
	thread_id: 스레드 번호
*/
void FilterLock(const unsigned int thread_id) {

    //while문 탈출 검사 변수
    bool while_exist = false;
    
    //대기실 레벨 1 부터 마지막 레벨까지 검사
    for (int i = 1; i < g_thread_num; i++) {
    
        //printf("%d level %d\n",thread_id, i);
        usleep(1);
	
        //자신을 다음 레벨의 대기실로 이동시킴
	//그 후, 현재 레벨의 양보 스레드를 자신으로 지정
        g_filter_level[thread_id] = i;
        g_filter_victim[i] = thread_id;

	//조건을 만족할 때까지 무한 루프
        while (while_exist) {
            while_exist = false;

	    //자신을 제외한 스레드가 자신보다 같거나 높은 단계에 존재하고 자신이 양보 스레드이면 대기
            for (int k = 1; k < g_thread_num; k++) {
                if (k != thread_id && g_filter_level[k] >= i && g_filter_victim[i] == thread_id) {
                    while_exist = true;
                    break;
                }
            }
        }
    }
}

/**
락 해제
인자: 
	thread_id: 스레드 번호
*/
void FilterUnlock(const unsigned int thread_id) {
    g_filter_level[thread_id] = 0;
}
