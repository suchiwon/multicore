/*
@FILE:		/include/global.h
@DATE:		2016/10/29
@AUTHOR:	��ġ��(suchiwon@hanyang.ac.kr)
@BRIEF:		���� ���� �� �ʱ�ȭ �Լ� ����
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
���� ��� �����
*/
const int RANDOM_MAX = 100;			        //�������� �ʱ� A,B���� ������ ���� �ִ밪 ����
const int GARBAGE_COLLECTOR_NUM = 1;		//������ �÷��� ������ ��
const int MODULAR = 100;		    	    //�����÷ο� ������ A,B �ִ� �߰� ��

/*
���� ��� ���� �����
*/
vector<long> g_sum_before_array;		    //�ʱ�ȭ �� �� �������� A,B �� ����� �迭
vector<long> g_sum_after_array;			    //��� �Ϸ� �� �� �������� A,B �� ����� �迭
vector<long> g_throughput;			        //�� �������� �ʴ� ��� Ƚ�� ����� �迭
vector<bool> g_verbose_input_flag;		    //�� �������� Ȯ�ο� ���� �Է� ��� �÷��� �迭

/*
���� ���� �����
*/
unsigned long g_execution_order = 0;		//���� Ȱ��ȭ �������� ����. lock ���ο��� atomic���� ����
unsigned long g_min_active_version = 0;		//���� Ȱ��ȭ ���� �� ���� ������ ����. ������ �ݷ��� ��

unsigned int g_thread_num;			        //�������� ����. ���� �� �����μ��� �Է� ����
unsigned int g_duration;			        //�����带 ������ �ð�.(����:��) ���� �� �����μ��� �Է� ����

bool g_b_verbose = false;			        //Ȯ�ο� ���� ���� ����. ���� �� �����μ�(verbose)�� �Է� ����
bool g_duration_end = false;			    //�ð� ����� �� �����忡�� ���� ���� �˸��� ����

void init();					            //���� ���� �ʱ�ȭ �Լ�

pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif // __PROJECT2_INCLUDE_GLOBAL_H__
