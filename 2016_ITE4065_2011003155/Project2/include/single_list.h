/*
@FILE:		/include/single_list.h
@DATE:		2016/10/29
@AUTHOR:	��ġ��(suchiwon@hanyang.ac.kr)
@BRIEF:		�̱� ����Ʈ�� ����� ��� �Լ� ����
*/
#ifndef __PROJECT2_INCLUDE_SINGLE_LIST_H__
#define __PROJECT2_INCLUDE_SINGLE_LIST_H__

#include <vector>
#include <stdlib.h>

using namespace std;

const long long NOT_FOUND = -1;			//������ ��ȣ�� ����Ʈ ���� ������ �˸��� ����

//����Ʈ�� ���� ���� ����ü
typedef struct _g_list_arg {
    long long A;				        //�� ������ A ����. �� �������� ���� ���忡���� ���
    long long B;				        //�� ������ B ����. �� �������� ���� ���忡���� ���
    unsigned int thread_id;			    //�������� ��ȣ. Ȱ�� ������ ����Ʈ���� ���
    unsigned long version;			    //���� ����Ʈ�� ����.
} g_list_arg;

//�̱� ����Ʈ�� ��� ����ü
typedef struct _g_single_list_node {
    g_list_arg argument;			    //���� ���� ����ü
    _g_single_list_node *p_next;		//���� ��� ���� ����
} g_single_list_node;

g_single_list_node *g_active_thread_list_head;			    //Ȱ��ȭ ������ ���� ����Ʈ ���
vector< g_single_list_node* > g_mvcc_thread_list_head_arr;	//�� �������� ���� ���� ����Ʈ ���

/**
�̱� ����Ʈ�� ��� �ʱ�ȭ
����:
	thread_num:	�������� ����
*/
void InitHead(int thread_num) {
	
    //Ȱ��ȭ ������ �ʱ�ȭ�� 
    g_active_thread_list_head = (g_single_list_node*)malloc(sizeof(g_single_list_node));
    g_active_thread_list_head->p_next = NULL;

    //���� ���� ����Ʈ ��� �迭 ũ�� ����
    g_mvcc_thread_list_head_arr.resize(thread_num);

    //�� �迭�� ��� �ʱ�ȭ
    for (int i = 0; i < thread_num; i++) {
        g_mvcc_thread_list_head_arr[i] = (g_single_list_node*)malloc(sizeof(g_single_list_node));
        g_mvcc_thread_list_head_arr[i]->p_next = NULL;
    }
}

/**
���ڷ� �־��� �̱� �������� �ʱ�ȭ
����:
	head: �̱� �������� ���
*/
void Init(g_single_list_node *head) {
    head = (g_single_list_node*)malloc(sizeof(g_single_list_node));
    head->p_next = NULL;
}
/**
�̱� �������� Ÿ�� ��� �ڿ� �� ��� �߰�
����:
	target: �ڽ� �ڿ� �߰��� Ÿ�� ���
	newVal: �߰��� �� ���
*/
void InsertNode(g_single_list_node *target, g_single_list_node *newVal) {
    g_single_list_node *temp;
    temp = (g_single_list_node*)malloc(sizeof(g_single_list_node));
    *temp = *newVal;
    
    temp->p_next = target->p_next;
    target->p_next = temp;
}

/**
�̱� �������� �� ��(����� �ٷ� ��)�� �� ��� �߰�
����:
	head: ��带 �߰��� �������� ���
	newVal: �߰��� �� ���
*/
void InsertNodeFront(g_single_list_node *head, g_single_list_node *newVal) {
    InsertNode(head, newVal);
}
/**
�� �������� mvcc�� �� ��� �߰�
����:
	thread_id: ��带 �߰��� mvcc ����Ʈ ����� �ε���
	newVal: �߰��� �� ���
*/
void InsertNodeMvccFront(const unsigned int thread_id, g_single_list_node *newVal) {
    InsertNodeFront(g_mvcc_thread_list_head_arr[thread_id], newVal);
}
/**
����Ʈ���� Ÿ�� ����� ���� ��� ����
����:
	target:	�ڽ��� ���� ��带 ������ Ÿ�� ���
*/
bool DeleteNode(g_single_list_node *target) {
    g_single_list_node *del;
    del = target->p_next;

    if (del == NULL) {
        return false;
    }

    target->p_next = del->p_next;
    free(del);
    
    return true;
}
/**
�������� readview���� ���õ� �����尡 �ִ��� Ȯ��. �� �������� ���� ��ȯ
����:
	head: �˻��� readview ����Ʈ�� ���
	thread_id: �˻��� readview�� ������ ��ȣ
*/
unsigned long FindReadView(g_single_list_node *head, const unsigned int thread_id) {
    g_single_list_node *search_node = head->p_next;

    while(search_node != NULL) {
        if (search_node->argument.thread_id == thread_id) {
            return search_node->argument.version;
        }
        search_node = search_node->p_next;
    }

	//readview�� �����尡 ������ ��� 0 ��ȯ
    return 0;
}
/**
������ �������� ���� ����Ʈ���� �о �Ǵ� ������ ã�� A���� ����
������ ������ ���� �ڽ��� ������Ʈ �������� ���� ���� �� ������Ʈ ���� ������ ������ ���� �ֽ� ����
����:
	thread_id: �˻��� ������ ��ȣ
	my_version: ���� ������Ʈ ���� �ڽ��� ����
	unsafe_version: ���� �� �����忡�� ������Ʈ ���̾ ������ �ȵǴ� ����. 0�̸� ���� ������ ����.
*/
pair<long long, unsigned long> FindSafeVersion( const unsigned int thread_id, 
                                                const unsigned long my_version, 
                                                const unsigned long unsafe_version) {
    g_single_list_node *search_node = g_mvcc_thread_list_head_arr[thread_id]->p_next;

    while (search_node != NULL) {
		
	    //Ž�� ���� ����Ʈ�� ������������ ���ĵǾ� �����Ƿ� �տ������� Ž���ϸ鼭 �ڽź��� ���� ������ ���ð��
	    //�� ������ �о�ߵǴ� �������� �Ǵ��Ѵ�
        if ((unsafe_version == 0 || search_node->argument.version != unsafe_version) &&
            search_node->argument.version < my_version) {
            
	        //������ ������ ã���� ��� �� ������ A���� ������ ��ȯ
            return make_pair(search_node->argument.A, search_node->argument.version);
        }

        search_node = search_node->p_next;
    }
	
    //������ ������ ���� ��� ã�� ���� ���� �˸� ���� ���
    return make_pair(NOT_FOUND, 0);
}
/**
�ڽ��� ������Ʈ�� ���� �� Ȱ��ȭ ����Ʈ���� �ڽ��� ����
����:
	thread_id: ������ �ڽ��� ������ ��ȣ
*/
void DeleteFromActiveList( const unsigned int thread_id) {
    g_single_list_node *search_node = g_active_thread_list_head;

    while (search_node->p_next != NULL) {
        if (search_node->p_next->argument.thread_id == thread_id) {
            DeleteNode(search_node);
            return;
        }
        search_node = search_node->p_next;
    }
}
/**
�� �������� ���� ����Ʈ���� �� �̻� ������ �ʴ� ���� ����
������ �ݷ��Ϳ��� ���.
���� Ȱ��ȭ ����Ʈ���� ���� ������ �������� ���� �������� Ž����
�� �Ʒ� ������ ���� ����
����:
	thread_id: ������ ����Ʈ�� ������ ��ȣ
	execute_version: ���� Ȱ��ȭ�� ������ ���� �� ���� ���� ��
*/
void DeleteUnreadVersion(   const unsigned int thread_id,
                            const unsigned long execute_version) {
    g_single_list_node *search_node = g_mvcc_thread_list_head_arr[thread_id]->p_next;

    int iter_count = 0;

    //���� ���� ����Ʈ���� �������� �ʾƾ��� �������� Ž��
    //�������� �ʾƾ��� ������ Ȱ��ȭ ����Ʈ ���� ���� ���� ���� ���� ū �����Ƿ�
    //������������ ���ĵ� ����Ʈ���� ���� ���� ���ö����� Ž����
    while (search_node != NULL) {
        if (search_node->argument.version < execute_version) {
            break;
        }
        search_node = search_node->p_next;
        iter_count++;
    }

    //������ ����Ʈ�� ũ�Ⱑ ����ϸ�
    //Ž���� ����Ʈ ����� ���� ���� ���� ����
    if (iter_count >= 2 && search_node != NULL) {
        
        while (DeleteNode(search_node)) {
        }
    }
}
/**
�� �̱� ����Ʈ�� �ٸ� �̱� ����Ʈ�� ����
Ȱ��ȭ ����Ʈ�� �ڽ��� readview�� �����ϴ� �뵵�� ���
���� ���� ������ ������ Ž������ ó���ϴ� �Լ��� ����ϹǷ� �� �Լ��� ������� ����
����:
	src_head: ������ ����Ʈ
	dest_head: ����� ���� ������ ����Ʈ
*/
void CopyList(g_single_list_node *src_head, g_single_list_node *dest_head) {
    g_single_list_node *target = src_head->p_next;

    //������ ����Ʈ�� �� �տ� �����ϹǷ� ����Ʈ�� �������� �����
    //Ȱ��ȭ ����Ʈ�� readview ����Ʈ�� ������ ��������Ƿ� ������ ����
    while (target != NULL) {
        InsertNodeFront(dest_head,target);
        target = target->p_next;
    }
}
/**
������ �ݷ��Ϳ��� ����� Ȱ��ȭ ������ �� ���� ���� ������ ã�� �Լ�
����� �Ʒ��� Ȱ��ȭ ����Ʈ�� ���� ����� ��ģ �Լ��� ���
����:
	head: Ž���� ����Ʈ�� ���
*/
unsigned long FindOldestActiveVersion(g_single_list_node *head) {
    g_single_list_node *search = head->p_next;
    unsigned long min_version = 0;

    if (search != NULL) {
        min_version = search->argument.version;   
    }

    while (search != NULL) {
        if (min_version > search->argument.version) {
            min_version = search->argument.version;
        }
        search = search->p_next;
    }

    return min_version;
}
/**
������ �ݷ����� �δ��� ���̱� ���� Ȱ��ȭ ����Ʈ�� ����� ���� ������ ������ Ž�� ����� ��ģ �Լ�
���� �� �����忡�� ���
����:
	src_head: ������ ����Ʈ
	dest_head: ����� ���� ������ ����Ʈ
*/
unsigned long CopyAndFindOldestVersion(g_single_list_node *src_head, g_single_list_node *dest_head) {
    g_single_list_node *target = src_head->p_next;

    unsigned long min_version = 0;

    if (target != NULL) {
        min_version = target->argument.version;   
    }

    while (target != NULL) {
        InsertNodeFront(dest_head, target);
        
        if (min_version > target->argument.version) {
            min_version = target->argument.version;
        }

        target = target->p_next;
    }

    return min_version;
}
/**
mvcc ����Ʈ �ʱ�ȭ
����:
	thread_id: �ʱ�ȭ�� mvcc ����Ʈ�� ������ ��ȣ
*/
void ClearMvccList(const unsigned int thread_id) {
    
    while (DeleteNode(g_mvcc_thread_list_head_arr[thread_id])) {
    }
}
/**
����Ʈ�� �ʱ�ȭ
����:
	head: �ʱ�ȭ�� ����Ʈ�� ���. ����� ����� ���� �����.
*/
void ClearList(g_single_list_node *head) {
    while (DeleteNode(head)) {
    }
}
/**
����Ʈ�� ��� ���� ��ȯ. ����� �������� ����
����:
	head: ������ �� ����Ʈ�� ���
*/
unsigned int GetListSize(g_single_list_node *head) {
    g_single_list_node *search = head->p_next;
    unsigned int size = 0;

    while (search != NULL) {
        size++;
        search = search->p_next;
    }

    return size;
}
/**
����Ʈ�� �� ����� ���� ���� ���
����:
	fp: ��� ���� �Է��� ������ ������
	head: ���� ������ Ž���� ����Ʈ�� ���
*/
void FprintfVersionInList(FILE* fp, g_single_list_node *head) {
    g_single_list_node *search = head->p_next;

    while (search != NULL) {
        fprintf(fp, "%ld ",search->argument.version);
        search = search->p_next;
    }
}

#endif //__PROJECT2_INCLUDE_SINGLE_LIST_H__
