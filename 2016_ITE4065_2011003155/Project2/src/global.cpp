/*
@FILE:		/src/global.cpp
@DATE:		2016/10/29
@AUTHOR:	서치원(suchiwon@hanyang.ac.kr)
@BRIEF:		global.h의 전역 변수 초기화 함수 정의
*/
#include "../include/global.h"
#include <iostream>
#include <stdlib.h>
/**
전역 배열 변수 초기화 함수
인자: --
*/
void Init() {
   
    InitHead(g_thread_num);		//싱글 리스트의 헤더 초기화 함수 호출
    
    //전역 배열 변수의 사이즈를 스레드 갯수로 초기화
    g_sum_before_array.resize(g_thread_num);	
    g_sum_after_array.resize(g_thread_num);
    g_throughput.resize(g_thread_num);
    g_verbose_input_flag.resize(g_thread_num);
}
