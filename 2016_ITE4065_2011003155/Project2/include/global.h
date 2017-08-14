/*
@FILE:		/include/global.h
@DATE:		2016/10/29
@AUTHOR:	서치원(suchiwon@hanyang.ac.kr)
@BRIEF:		전역 변수 및 초기화 함수 선언
*/
#ifndef __PROJECT2_INCLUDE_GLOBAL_H__
#define __PROJECT2_INCLUDE_GLOBAL_H__

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <string>
#include <vector>
#include <pthread.h>
#include "single_list.h"

/*
전역 상수 선언부
*/
const int RANDOM_MAX = 100;			        //스레드의 초기 A,B값의 랜덤값 생성 최대값 변수
const int GARBAGE_COLLECTOR_NUM = 1;		//가비지 컬렉션 스레드 수
const int MODULAR = 100;		    	    //오버플로우 방지용 A,B 최대 추가 값

/*
전역 행렬 변수 선언부
*/
vector<long> g_sum_before_array;		    //초기화 된 각 스레드의 A,B 합 저장용 배열
vector<long> g_sum_after_array;			    //계산 완료 후 각 스레드의 A,B 합 저장용 배열
vector<long> g_throughput;			        //각 스레드의 초당 계산 횟수 저장용 배열
vector<bool> g_verbose_input_flag;		    //각 스레드의 확인용 파일 입력 명령 플래그 배열

/*
전역 변수 선언부
*/
unsigned long g_execution_order = 0;		//현재 활성화 스레드의 순서. lock 내부에서 atomic으로 증가
unsigned long g_min_active_version = 0;		//현재 활성화 버전 중 가장 오래된 버전. 가비지 콜렉션 용

unsigned int g_thread_num;			        //스레드의 갯수. 실행 시 가변인수로 입력 받음
unsigned int g_duration;			        //스레드를 실행할 시간.(단위:초) 실행 시 가변인수로 입력 받음

bool g_b_verbose = false;			        //확인용 파일 생성 여부. 실행 시 가변인수(verbose)로 입력 받음
bool g_duration_end = false;			    //시간 종료시 각 스레드에게 종료 여부 알림용 변수

void init();					            //전역 변수 초기화 함수

pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif // __PROJECT2_INCLUDE_GLOBAL_H__
