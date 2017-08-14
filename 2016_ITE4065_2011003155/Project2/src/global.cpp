/*
@FILE:		/src/global.cpp
@DATE:		2016/10/29
@AUTHOR:	��ġ��(suchiwon@hanyang.ac.kr)
@BRIEF:		global.h�� ���� ���� �ʱ�ȭ �Լ� ����
*/
#include "../include/global.h"
#include <iostream>
#include <stdlib.h>
/**
���� �迭 ���� �ʱ�ȭ �Լ�
����: --
*/
void Init() {
   
    InitHead(g_thread_num);		//�̱� ����Ʈ�� ��� �ʱ�ȭ �Լ� ȣ��
    
    //���� �迭 ������ ����� ������ ������ �ʱ�ȭ
    g_sum_before_array.resize(g_thread_num);	
    g_sum_after_array.resize(g_thread_num);
    g_throughput.resize(g_thread_num);
    g_verbose_input_flag.resize(g_thread_num);
}
