/*
@FILE:		/src/bakery_lock.cpp
@DATE:		2016/10/29
@AUTHOR:	서치원(suchiwon@hanyang.ac.kr)
@BRIEF:		각 스레드의 활성화 리스트 변경의 충돌 방지용 락
*/
#include "../include/global.h"
#include <unistd.h>
#include <stdlib.h>

vector<bool> g_lock_flag;			    //스레드의 락 진입 여부
vector<unsigned long> g_lock_label;		    //스레드의 락 진입 순서
unsigned long g_lock_counter;
/**
락 변수 초기화
인자: --
*/
void LockInit() {

    //스레드 갯수만큼 배열 크기 지정
    g_lock_flag.resize(g_thread_num);
    g_lock_label.resize(g_thread_num);

    for (int i = 0; i < g_thread_num; i++) {
        g_lock_flag[i] = false;
        g_lock_label[i] = 0;
    }

    g_lock_counter = 0;
}
/**
현재 대기 번호중 최대 값 반환. 현재는 사용하지 않음.
인자: --
*/
unsigned long GetMaxLabel() {
    unsigned long max = 0;

    for (int i = 0; i < g_thread_num; i++) {
        if (max < g_lock_label[i]) {
            max = g_lock_label[i];
        }
    }
    return max;
}
/**
락 설정.delay를 제거하고 2개 이상의 스레드에서 실행시 에러가 남.
인자:
	thread_id: 락을 설정한 스레드 번호
*/
void Lock(const unsigned int thread_id) {

//현재 사용 베이커리 락 방식

    //대기 플래그 활성화
    g_lock_flag[thread_id] = true;

    //대기표를 순번이 겹치지 않게 선택
    g_lock_label[thread_id] = __sync_fetch_and_add(&g_lock_counter,1);
    
    bool while_check = true;

    //자신 앞에 대기하고 있는 스레드가 없을 때까지 루프
    while (true) {
        while_check = false;

	//자신이 아니면서 대기 중이고 자신 보다 대기번호가 낮은 스레드가 있으면 대기
        for (int i = 0; i < g_thread_num; i++) {
            if (    i != thread_id && 
                    g_lock_flag[i] && 
                    (   g_lock_label[thread_id] > g_lock_label[i] || 
                        (g_lock_label[thread_id] == g_lock_label[i] && thread_id > i)
                    )) {
                while_check = true;
                break;
            }
        }

	//대기 중일 경우 에러 방지를 위해 delay 적용
        if (while_check == false) {
            break;
        } else {
            //pthread_yield();
            usleep(1);
            //printf("test\n");
        }
    }


//이전 사용 베이커리 락 방식
/*
    //자신의 대기표 플래그 활성화
    __sync_synchronize();
    g_lock_flag[thread_id] = true;
    __sync_synchronize();
    unsigned long max = 0;

    for (int i = 0; i < g_thread_num; i++) {
        if (max < g_lock_label[i]) {
            max = g_lock_label[i];
        }
    }
    
    //대기 번호를 현재 대기 번호 중 가장 큰값에 1을 더해 가짐(마지막 대기)
    g_lock_label[thread_id] = max + 1;

    //자신의 대기표 플래그 비활성화
    __sync_synchronize();
    g_lock_flag[thread_id] = false;
    __sync_synchronize();
    
    //스레드 갯수만큼 자신보다 먼저 대기 중인 스레드가 있는지 확인
    for (int i = 0; i < g_thread_num; i++) {
        //usleep(2);
        printf("%d test %d\n",thread_id,i);
	    //이 스레드가 대기표를 발권 중인지 확인
        while (g_lock_flag[i]) {
        }

        //이 스레드가 발권했을 경우 자신보다 먼저 대기 했을 경우 대기. 락을 종료시 루프를 빠져나옴.
	while(true) {    
            if (g_lock_label[i] == 0) {
                break;
            } 
            
            if (g_lock_label[thread_id] < g_lock_label[i]) {
                break;
            } 
            
	    //만약 같은 대기번호를 가졌을 경우 스레드 번호가 낮은 쪽이 우선권을 가짐
            if (g_lock_label[thread_id] == g_lock_label[i]) {
                if (thread_id <= i) {
                    break;
                }
            }
        }
    }
*/
}

/**
락 해제
인자:
	thread_id: 락을 설정한 스레드 번호
*/
void Unlock(unsigned int thread_id) {
    g_lock_flag[thread_id] = false;

    //자신의 대기표를 버려서 종료되었음을 알림
    //g_lock_label[thread_id] = 0;
}
