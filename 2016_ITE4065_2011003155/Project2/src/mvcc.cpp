/*
@FILE:		/src/mvcc.cpp
@DATE:		2016/10/29
@AUTHOR:	��ġ��(suchiwon@hanyang.ac.kr)
@BRIEF:		�� �����忡�� ���� ����� ���� �Լ� ����
*/
#include "../include/global.h"
#include "bakery_lock.cpp"
#include "filter_lock.cpp"
#include <string>
#include <stdlib.h>
#include <iostream>

/**
�������� ���� �Լ�
����:
	arg: main�Լ����� �Ѿ�� ����. ������ ��ȣ.
*/
void *ThreadFuncMvcc(void* arg) {
    int thread_id = *((int*)arg);

    //�������� readview ����Ʈ ���
    g_single_list_node *read_view_list_head;

    //readview ����� �ʱ�ȭ
    read_view_list_head = (g_single_list_node*)malloc(sizeof(g_single_list_node));
    read_view_list_head->p_next = NULL;
    //Init(read_view_list_head);

    FILE *p_file_verbose;		//verbose ���� ������
	
    unsigned int picked_thread;		//���� ������Ʈ���� ���õ� ������ ��ȣ. �������� ����
    unsigned long oldest_version;	//���� ������Ʈ���� Ȱ��ȭ ����Ʈ �� ���� ������ ����. ������ �ݷ��Ϳ��� ���.

    char file_name[255];
    sprintf(file_name, "verbose_thread_%d.txt",thread_id);

    //verbose ����� �Է� �Ǿ��� ��� ���� �ҷ���.
    if (g_b_verbose) {
        p_file_verbose = fopen(file_name,"w");
    }

    //������Ʈ �� �ڽ��� �ֽ� ����. �ڽ��� �� ��带 ����.
    g_single_list_node update_version;

    //A, B���� �������� �����ϰ� ������ 0���� �ʱ�ȭ
    update_version.argument.A = rand()%RANDOM_MAX;
    update_version.argument.B = rand()%RANDOM_MAX;
    update_version.argument.thread_id = thread_id;
    update_version.argument.version = 0;

    //���� ������Ʈ ���� �ڽ��� ����
    unsigned long updating_version;
    //������ ������ �� ������ ������ ����. first: A ��, second: ������ ����
    pair<long long, unsigned long> picked_version;

    //��� �� A,B�� �� ����
    g_sum_before_array[thread_id] = update_version.argument.A + update_version.argument.B;

    //���� ���� ����Ʈ�� ������ ���� ����
    InsertNodeMvccFront(thread_id, &update_version);

    //printf("thread %d before   A:%lld  B:%lld\n",thread_id,update_version.argument.A,update_version.argument.B);
    
	//���� �Լ��� Ÿ�̸� ������ ����� �� ���� ����
	while (!g_duration_end) {

	    //readview ����Ʈ�� �ʱ�ȭ
        ClearList(read_view_list_head);
        
	    //Ȱ��ȭ ����Ʈ �������� �浹�� �����ϱ� ���� �� ����
        pthread_mutex_lock(&g_mutex);
        //Lock(thread_id);
        //FilterLock(thread_id);

	        //������Ʈ ���� ���� �� �ڽ��� ������Ʈ �������� ������
            g_execution_order++;
            update_version.argument.version = g_execution_order;

	        //Ȱ��ȭ ����Ʈ�� �ڽ� ��� �� readview�� �����ϸ鼭 ���� ������ ���� Ž��
            InsertNodeFront(g_active_thread_list_head, &update_version);
            oldest_version = CopyAndFindOldestVersion(g_active_thread_list_head, read_view_list_head);

            if (g_min_active_version < oldest_version) {
                g_min_active_version = oldest_version;
            }

	    //�� ����
        pthread_mutex_unlock(&g_mutex);
        //Unlock(thread_id);
        //FilterUnlock(thread_id);

	    //������Ʈ�� A���� �о�� ������ ����.
        picked_thread = rand()%g_thread_num;

	    //�� �����尡 ������Ʈ ������ Ȯ��
        updating_version = FindReadView(read_view_list_head, picked_thread);
        
		//�� �������� ���� ����Ʈ���� ������ �� ������
		picked_version = FindSafeVersion(picked_thread, update_version.argument.version, updating_version);
        
		//������ A ���� ���
		if (picked_version.first != NOT_FOUND) {
            update_version.argument.A += picked_version.first%MODULAR;
            update_version.argument.B -= picked_version.first%MODULAR;
        }

	    //�ڽ��� �ֽ� ���� �߰�
        InsertNodeMvccFront(thread_id, &update_version);

	    //Ȱ��ȭ ����Ʈ���� �ڽ��� �����ϱ� ���� �� ����
        pthread_mutex_lock(&g_mutex);
        //Lock(thread_id);
        //FilterLock(thread_id);
	        //Ȱ��ȭ ����Ʈ���� �ڽ� ����
            DeleteFromActiveList(thread_id);
        

	    //�� ����
        pthread_mutex_unlock(&g_mutex);
        //Unlock(thread_id);  
        //FilterUnlock(thread_id);

	    //verbose ����� ����Ǿ��� 0.1�ʰ� ���� ���� �Է��� �ؾ��� ��� ���� ���� �Է�
        if (g_b_verbose && g_verbose_input_flag[thread_id]) {
            fprintf(p_file_verbose,"%d\n",GetListSize(read_view_list_head));
            
            FprintfVersionInList(p_file_verbose, read_view_list_head);
            fprintf(p_file_verbose,"\n");

            fprintf(p_file_verbose,"%ld\n",picked_version.second);

            g_verbose_input_flag[thread_id] = false;
        }

	    //������Ʈ ���� Ƚ�� ����
        g_throughput[thread_id]++;

        //printf("%lld %lld\n",update_version.A,update_version.B);
    }
    
    //verbose ���� ����
    if (g_b_verbose) {
        fclose(p_file_verbose);
    }

    //�ʴ� throughput ���
    g_throughput[thread_id] /= g_duration;

    //��� �� A,B ���� �� ����
    //printf("thread %d after   A:%lld  B:%lld\n",thread_id,update_version.argument.A,update_version.argument.B);
    g_sum_after_array[thread_id] = update_version.argument.A + update_version.argument.B;

    return NULL;
}
