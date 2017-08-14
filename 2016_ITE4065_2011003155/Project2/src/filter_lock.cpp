/*
@FILE:		/src/filter_lock.cpp
@DATE:		2016/10/29
@AUTHOR:	��ġ��(suchiwon@hanyang.ac.kr)
@BRIEF:		�� �������� Ȱ��ȭ ����Ʈ ������ �浹 ������ ��
*/
#include "../include/global.h"
#include <unistd.h>

vector<int> g_filter_level;		//�� �ܰ��� ���� �迭
vector<int> g_filter_victim;		//�� �ܰ��� �纸 ������ ���� �迭

/**
�� ���� �ʱ�ȭ
����: 
	thread_num: �������� ����
*/
void FilterLockInit(const int thread_num) {
    g_filter_level.resize(thread_num);
    g_filter_victim.resize(thread_num);
}

/**
�� ����
����: 
	thread_id: ������ ��ȣ
*/
void FilterLock(const unsigned int thread_id) {

    //while�� Ż�� �˻� ����
    bool while_exist = false;
    
    //���� ���� 1 ���� ������ �������� �˻�
    for (int i = 1; i < g_thread_num; i++) {
    
        //printf("%d level %d\n",thread_id, i);
        usleep(1);
	
        //�ڽ��� ���� ������ ���Ƿ� �̵���Ŵ
	//�� ��, ���� ������ �纸 �����带 �ڽ����� ����
        g_filter_level[thread_id] = i;
        g_filter_victim[i] = thread_id;

	//������ ������ ������ ���� ����
        while (while_exist) {
            while_exist = false;

	    //�ڽ��� ������ �����尡 �ڽź��� ���ų� ���� �ܰ迡 �����ϰ� �ڽ��� �纸 �������̸� ���
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
�� ����
����: 
	thread_id: ������ ��ȣ
*/
void FilterUnlock(const unsigned int thread_id) {
    g_filter_level[thread_id] = 0;
}
