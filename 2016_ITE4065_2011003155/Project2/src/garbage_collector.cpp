/*
@FILE:		/src/gabage_collector.cpp
@DATE:		2016/10/29
@AUTHOR:	��ġ��(suchiwon@hanyang.ac.kr)
@BRIEF:		�� �������� ���� ����Ʈ�� �޸� �����÷ο� ������ ������ �ݷ���
*/
#include "../include/global.h"

/**
�������� ���� �Լ�
����:
	arg: main�Լ����� �Ѿ�� ����. ������ �ݷ��� ������ ��ȣ.
*/
void* ThreadFuncGarbageCollector(void* arg) {
    
    int b_collect_min_version = (*(int*)arg);

    unsigned long version_temp;

    //���� �Լ� Ÿ�̸� ���� ����� ���� ����
    while (!g_duration_end) {

	//������ �ݷ��� 0���� Ȱ��ȭ ����Ʈ �� ���� ������ ������ Ž��������
	//���� �� ������ �� ��� �����尡 ������
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

    	//�� �����带 ���鼭 �� �̻� ���� �ʴ� ������ ����
        for (int i = 0; i < g_thread_num; i++) {
            DeleteUnreadVersion(i, g_min_active_version);    
        }
    }

    return NULL;
}
