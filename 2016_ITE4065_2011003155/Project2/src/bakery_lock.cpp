/*
@FILE:		/src/bakery_lock.cpp
@DATE:		2016/10/29
@AUTHOR:	��ġ��(suchiwon@hanyang.ac.kr)
@BRIEF:		�� �������� Ȱ��ȭ ����Ʈ ������ �浹 ������ ��
*/
#include "../include/global.h"
#include <unistd.h>
#include <stdlib.h>

vector<bool> g_lock_flag;			    //�������� �� ���� ����
vector<unsigned long> g_lock_label;		    //�������� �� ���� ����
unsigned long g_lock_counter;
/**
�� ���� �ʱ�ȭ
����: --
*/
void LockInit() {

    //������ ������ŭ �迭 ũ�� ����
    g_lock_flag.resize(g_thread_num);
    g_lock_label.resize(g_thread_num);

    for (int i = 0; i < g_thread_num; i++) {
        g_lock_flag[i] = false;
        g_lock_label[i] = 0;
    }

    g_lock_counter = 0;
}
/**
���� ��� ��ȣ�� �ִ� �� ��ȯ. ����� ������� ����.
����: --
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
�� ����.delay�� �����ϰ� 2�� �̻��� �����忡�� ����� ������ ��.
����:
	thread_id: ���� ������ ������ ��ȣ
*/
void Lock(const unsigned int thread_id) {

//���� ��� ����Ŀ�� �� ���

    //��� �÷��� Ȱ��ȭ
    g_lock_flag[thread_id] = true;

    //���ǥ�� ������ ��ġ�� �ʰ� ����
    g_lock_label[thread_id] = __sync_fetch_and_add(&g_lock_counter,1);
    
    bool while_check = true;

    //�ڽ� �տ� ����ϰ� �ִ� �����尡 ���� ������ ����
    while (true) {
        while_check = false;

	//�ڽ��� �ƴϸ鼭 ��� ���̰� �ڽ� ���� ����ȣ�� ���� �����尡 ������ ���
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

	//��� ���� ��� ���� ������ ���� delay ����
        if (while_check == false) {
            break;
        } else {
            //pthread_yield();
            usleep(1);
            //printf("test\n");
        }
    }


//���� ��� ����Ŀ�� �� ���
/*
    //�ڽ��� ���ǥ �÷��� Ȱ��ȭ
    __sync_synchronize();
    g_lock_flag[thread_id] = true;
    __sync_synchronize();
    unsigned long max = 0;

    for (int i = 0; i < g_thread_num; i++) {
        if (max < g_lock_label[i]) {
            max = g_lock_label[i];
        }
    }
    
    //��� ��ȣ�� ���� ��� ��ȣ �� ���� ū���� 1�� ���� ����(������ ���)
    g_lock_label[thread_id] = max + 1;

    //�ڽ��� ���ǥ �÷��� ��Ȱ��ȭ
    __sync_synchronize();
    g_lock_flag[thread_id] = false;
    __sync_synchronize();
    
    //������ ������ŭ �ڽź��� ���� ��� ���� �����尡 �ִ��� Ȯ��
    for (int i = 0; i < g_thread_num; i++) {
        //usleep(2);
        printf("%d test %d\n",thread_id,i);
	    //�� �����尡 ���ǥ�� �߱� ������ Ȯ��
        while (g_lock_flag[i]) {
        }

        //�� �����尡 �߱����� ��� �ڽź��� ���� ��� ���� ��� ���. ���� ����� ������ ��������.
	while(true) {    
            if (g_lock_label[i] == 0) {
                break;
            } 
            
            if (g_lock_label[thread_id] < g_lock_label[i]) {
                break;
            } 
            
	    //���� ���� ����ȣ�� ������ ��� ������ ��ȣ�� ���� ���� �켱���� ����
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
�� ����
����:
	thread_id: ���� ������ ������ ��ȣ
*/
void Unlock(unsigned int thread_id) {
    g_lock_flag[thread_id] = false;

    //�ڽ��� ���ǥ�� ������ ����Ǿ����� �˸�
    //g_lock_label[thread_id] = 0;
}
