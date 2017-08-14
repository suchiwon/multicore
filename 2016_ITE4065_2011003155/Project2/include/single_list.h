/*
@FILE:		/include/single_list.h
@DATE:		2016/10/29
@AUTHOR:	서치원(suchiwon@hanyang.ac.kr)
@BRIEF:		싱글 리스트의 헤더와 사용 함수 선언
*/
#ifndef __PROJECT2_INCLUDE_SINGLE_LIST_H__
#define __PROJECT2_INCLUDE_SINGLE_LIST_H__

#include <vector>
#include <stdlib.h>

using namespace std;

const long long NOT_FOUND = -1;			//스레드 번호가 리스트 내에 없음을 알리는 변수

//리스트의 내부 변수 구조체
typedef struct _g_list_arg {
    long long A;				        //각 버전의 A 숫자. 각 스레드의 버전 저장에서만 사용
    long long B;				        //각 버전의 B 숫자. 각 스레드의 버전 저장에서만 사용
    unsigned int thread_id;			    //스레드의 번호. 활성 스레드 리스트에서 사용
    unsigned long version;			    //현재 리스트의 버전.
} g_list_arg;

//싱글 리스트의 노드 구조체
typedef struct _g_single_list_node {
    g_list_arg argument;			    //내부 변수 구조체
    _g_single_list_node *p_next;		//다음 노드 저장 변수
} g_single_list_node;

g_single_list_node *g_active_thread_list_head;			    //활성화 스레드 저장 리스트 헤드
vector< g_single_list_node* > g_mvcc_thread_list_head_arr;	//각 스레드의 버전 저장 리스트 헤드

/**
싱글 리스트의 헤더 초기화
인자:
	thread_num:	스레드의 갯수
*/
void InitHead(int thread_num) {
	
    //활성화 스레드 초기화부 
    g_active_thread_list_head = (g_single_list_node*)malloc(sizeof(g_single_list_node));
    g_active_thread_list_head->p_next = NULL;

    //버전 관리 리스트 헤드 배열 크기 설정
    g_mvcc_thread_list_head_arr.resize(thread_num);

    //각 배열의 헤드 초기화
    for (int i = 0; i < thread_num; i++) {
        g_mvcc_thread_list_head_arr[i] = (g_single_list_node*)malloc(sizeof(g_single_list_node));
        g_mvcc_thread_list_head_arr[i]->p_next = NULL;
    }
}

/**
인자로 주어진 싱글 스레드의 초기화
인자:
	head: 싱글 스레드의 헤드
*/
void Init(g_single_list_node *head) {
    head = (g_single_list_node*)malloc(sizeof(g_single_list_node));
    head->p_next = NULL;
}
/**
싱글 스레드의 타겟 노드 뒤에 새 노드 추가
인자:
	target: 자신 뒤에 추가될 타겟 노드
	newVal: 추가될 새 노드
*/
void InsertNode(g_single_list_node *target, g_single_list_node *newVal) {
    g_single_list_node *temp;
    temp = (g_single_list_node*)malloc(sizeof(g_single_list_node));
    *temp = *newVal;
    
    temp->p_next = target->p_next;
    target->p_next = temp;
}

/**
싱글 스레드의 맨 앞(헤드의 바로 뒤)에 새 노드 추가
인자:
	head: 노드를 추가할 스레드의 헤드
	newVal: 추가될 새 노드
*/
void InsertNodeFront(g_single_list_node *head, g_single_list_node *newVal) {
    InsertNode(head, newVal);
}
/**
한 스레드의 mvcc에 새 노드 추가
인자:
	thread_id: 노드를 추가할 mvcc 리스트 헤드의 인덱스
	newVal: 추가될 새 노드
*/
void InsertNodeMvccFront(const unsigned int thread_id, g_single_list_node *newVal) {
    InsertNodeFront(g_mvcc_thread_list_head_arr[thread_id], newVal);
}
/**
리스트에서 타겟 노드의 다음 노드 제거
인자:
	target:	자신의 다음 노드를 제거할 타겟 노드
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
스레드의 readview에서 선택된 스레드가 있는지 확인. 그 스레드의 버전 반환
인자:
	head: 검색할 readview 리스트의 헤드
	thread_id: 검색할 readview의 스레드 번호
*/
unsigned long FindReadView(g_single_list_node *head, const unsigned int thread_id) {
    g_single_list_node *search_node = head->p_next;

    while(search_node != NULL) {
        if (search_node->argument.thread_id == thread_id) {
            return search_node->argument.version;
        }
        search_node = search_node->p_next;
    }

	//readview에 스레드가 없었을 경우 0 반환
    return 0;
}
/**
선택한 스레드의 버전 리스트에서 읽어도 되는 버전을 찾아 A값을 읽음
안전한 버전은 현재 자신의 업데으트 버전보다 낮은 버전 중 업데이트 중인 버전을 제외한 가장 최신 버전
인자:
	thread_id: 검색할 스레드 번호
	my_version: 현재 업데이트 중인 자신의 버전
	unsafe_version: 현재 이 스레드에서 업데이트 중이어서 읽으면 안되는 버전. 0이면 없는 것으로 설정.
*/
pair<long long, unsigned long> FindSafeVersion( const unsigned int thread_id, 
                                                const unsigned long my_version, 
                                                const unsigned long unsafe_version) {
    g_single_list_node *search_node = g_mvcc_thread_list_head_arr[thread_id]->p_next;

    while (search_node != NULL) {
		
	    //탐색 중인 리스트는 내림차순으로 정렬되어 있으므로 앞에서부터 탐색하면서 자신보다 낮은 버전이 나올경우
	    //그 버전을 읽어야되는 버전으로 판단한다
        if ((unsafe_version == 0 || search_node->argument.version != unsafe_version) &&
            search_node->argument.version < my_version) {
            
	        //안전한 버전을 찾았을 경우 그 버전의 A값과 버전을 반환
            return make_pair(search_node->argument.A, search_node->argument.version);
        }

        search_node = search_node->p_next;
    }
	
    //안전한 버전이 없을 경우 찾지 못한 것을 알릴 값을 출력
    return make_pair(NOT_FOUND, 0);
}
/**
자신의 업데이트가 끝난 후 활성화 리스트에서 자신을 제거
인자:
	thread_id: 제거할 자신의 스레드 번호
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
각 스레드의 버전 리스트에서 더 이상 읽히지 않는 노드들 제거
가비지 콜렉터에서 사용.
현재 활성화 리스트에서 가장 오래된 버전보다 작은 버전까지 탐색후
그 아래 버전을 전부 삭제
인자:
	thread_id: 제거할 리스트의 스레드 번호
	execute_version: 현재 활성화된 스레드 버전 중 가장 낮은 값
*/
void DeleteUnreadVersion(   const unsigned int thread_id,
                            const unsigned long execute_version) {
    g_single_list_node *search_node = g_mvcc_thread_list_head_arr[thread_id]->p_next;

    int iter_count = 0;

    //현재 버전 리스트에서 삭제되지 않아야할 버전까지 탐색
    //삭제되지 않아야할 버전은 활성화 리스트 버전 보다 낮은 값중 가장 큰 값으므로
    //내림차순으로 정렬된 리스트에서 낮은 값이 나올때까지 탐색함
    while (search_node != NULL) {
        if (search_node->argument.version < execute_version) {
            break;
        }
        search_node = search_node->p_next;
        iter_count++;
    }

    //삭제할 리스트의 크기가 충분하면
    //탐색한 리스트 노드의 다음 값을 전부 삭제
    if (iter_count >= 2 && search_node != NULL) {
        
        while (DeleteNode(search_node)) {
        }
    }
}
/**
한 싱글 리스트를 다른 싱글 리스트에 복사
활성화 리스트를 자신의 readview로 복사하는 용도로 사용
현재 가장 오래된 버전의 탐색까지 처리하는 함수를 사용하므로 이 함수는 사용하지 않음
인자:
	src_head: 복사할 리스트
	dest_head: 복사된 값을 저장할 리스트
*/
void CopyList(g_single_list_node *src_head, g_single_list_node *dest_head) {
    g_single_list_node *target = src_head->p_next;

    //저장은 리스트의 맨 앞에 저장하므로 리스트가 뒤집혀서 저장됨
    //활성화 리스트와 readview 리스트는 순서에 상관없으므로 문제가 없음
    while (target != NULL) {
        InsertNodeFront(dest_head,target);
        target = target->p_next;
    }
}
/**
가비지 콜렉터에서 사용할 활성화 스레드 중 가장 낮은 버전을 찾는 함수
현재는 아래의 활성화 리스트의 복사 기능을 합친 함수를 사용
인자:
	head: 탐색할 리스트의 헤드
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
가비지 콜렉터의 부담을 줄이기 위해 활성화 리스트의 복사와 가장 오래된 버전의 탐색 기능을 합친 함수
현재 각 스레드에서 사용
인자:
	src_head: 복사할 리스트
	dest_head: 복사된 값을 저장할 리스트
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
mvcc 리스트 초기화
인자:
	thread_id: 초기화할 mvcc 리스트의 스레드 번호
*/
void ClearMvccList(const unsigned int thread_id) {
    
    while (DeleteNode(g_mvcc_thread_list_head_arr[thread_id])) {
    }
}
/**
리스트의 초기화
인자:
	head: 초기화할 리스트의 헤더. 헤더만 남기고 전부 지운다.
*/
void ClearList(g_single_list_node *head) {
    while (DeleteNode(head)) {
    }
}
/**
리스트의 노드 갯수 반환. 헤더는 갯수에서 제외
인자:
	head: 갯수를 셀 리스트의 헤드
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
리스트의 각 노드의 버전 변수 출력
인자:
	fp: 출력 값을 입력할 파일의 포인터
	head: 버전 변수를 탐색할 리스트의 헤드
*/
void FprintfVersionInList(FILE* fp, g_single_list_node *head) {
    g_single_list_node *search = head->p_next;

    while (search != NULL) {
        fprintf(fp, "%ld ",search->argument.version);
        search = search->p_next;
    }
}

#endif //__PROJECT2_INCLUDE_SINGLE_LIST_H__
