/*
@FILE:		/src/mvcc.cpp
@DATE:		2016/10/29
@AUTHOR:	서치원(suchiwon@hanyang.ac.kr)
@BRIEF:		각 스레드에서 병렬 계산을 위한 함수 정의
*/
#include "../include/global.h"
#include "bakery_lock.cpp"
#include "filter_lock.cpp"
#include <string>
#include <stdlib.h>
#include <iostream>

/**
스레드의 실행 함수
인자:
	arg: main함수에서 넘어온 인자. 스레드 번호.
*/
void *ThreadFuncMvcc(void* arg) {
    int thread_id = *((int*)arg);

    //스레드의 readview 리스트 헤드
    g_single_list_node *read_view_list_head;

    //readview 헤드의 초기화
    read_view_list_head = (g_single_list_node*)malloc(sizeof(g_single_list_node));
    read_view_list_head->p_next = NULL;
    //Init(read_view_list_head);

    FILE *p_file_verbose;		//verbose 파일 포인터
	
    unsigned int picked_thread;		//현재 업데이트에서 선택된 스레드 번호. 랜덤으로 지정
    unsigned long oldest_version;	//현재 업데이트에서 활성화 리스트 중 가장 오래된 버전. 가비지 콜렉터에서 사용.

    char file_name[255];
    sprintf(file_name, "verbose_thread_%d.txt",thread_id);

    //verbose 명령이 입력 되었을 경우 파일 불러옴.
    if (g_b_verbose) {
        p_file_verbose = fopen(file_name,"w");
    }

    //업데이트 할 자신의 최신 버전. 자신은 이 노드를 참조.
    g_single_list_node update_version;

    //A, B값을 랜덤으로 생성하고 버전을 0으로 초기화
    update_version.argument.A = rand()%RANDOM_MAX;
    update_version.argument.B = rand()%RANDOM_MAX;
    update_version.argument.thread_id = thread_id;
    update_version.argument.version = 0;

    //현재 업데이트 중인 자신의 버전
    unsigned long updating_version;
    //선택한 스레드 중 가져온 안전한 버전. first: A 값, second: 가져온 버전
    pair<long long, unsigned long> picked_version;

    //계산 전 A,B의 합 저장
    g_sum_before_array[thread_id] = update_version.argument.A + update_version.argument.B;

    //버전 저장 리스트에 최초의 버전 저장
    InsertNodeMvccFront(thread_id, &update_version);

    //printf("thread %d before   A:%lld  B:%lld\n",thread_id,update_version.argument.A,update_version.argument.B);
    
	//메인 함수의 타이머 루프가 종료될 때 까지 실행
	while (!g_duration_end) {

	    //readview 리스트의 초기화
        ClearList(read_view_list_head);
        
	    //활성화 리스트 변형에서 충돌을 방지하기 위해 락 설정
        pthread_mutex_lock(&g_mutex);
        //Lock(thread_id);
        //FilterLock(thread_id);

	        //업데이트 버전 증가 후 자신의 업데이트 버전으로 가져옴
            g_execution_order++;
            update_version.argument.version = g_execution_order;

	        //활성화 리스트에 자신 등록 후 readview로 복사하면서 가장 오래된 버전 탐색
            InsertNodeFront(g_active_thread_list_head, &update_version);
            oldest_version = CopyAndFindOldestVersion(g_active_thread_list_head, read_view_list_head);

            if (g_min_active_version < oldest_version) {
                g_min_active_version = oldest_version;
            }

	    //락 해제
        pthread_mutex_unlock(&g_mutex);
        //Unlock(thread_id);
        //FilterUnlock(thread_id);

	    //업데이트할 A값을 읽어올 스레드 지정.
        picked_thread = rand()%g_thread_num;

	    //그 스레드가 업데이트 중인지 확인
        updating_version = FindReadView(read_view_list_head, picked_thread);
        
		//그 스레드의 버전 리스트에서 안전한 값 가져옴
		picked_version = FindSafeVersion(picked_thread, update_version.argument.version, updating_version);
        
		//가져온 A 값을 계산
		if (picked_version.first != NOT_FOUND) {
            update_version.argument.A += picked_version.first%MODULAR;
            update_version.argument.B -= picked_version.first%MODULAR;
        }

	    //자신의 최신 버전 추가
        InsertNodeMvccFront(thread_id, &update_version);

	    //활성화 리스트에서 자신을 제거하기 위해 락 설정
        pthread_mutex_lock(&g_mutex);
        //Lock(thread_id);
        //FilterLock(thread_id);
	        //활성화 리스트에서 자신 제거
            DeleteFromActiveList(thread_id);
        

	    //락 해제
        pthread_mutex_unlock(&g_mutex);
        //Unlock(thread_id);  
        //FilterUnlock(thread_id);

	    //verbose 명령이 실행되었고 0.1초가 지나 파일 입력을 해야할 경우 현재 상태 입력
        if (g_b_verbose && g_verbose_input_flag[thread_id]) {
            fprintf(p_file_verbose,"%d\n",GetListSize(read_view_list_head));
            
            FprintfVersionInList(p_file_verbose, read_view_list_head);
            fprintf(p_file_verbose,"\n");

            fprintf(p_file_verbose,"%ld\n",picked_version.second);

            g_verbose_input_flag[thread_id] = false;
        }

	    //업데이트 실행 횟수 증가
        g_throughput[thread_id]++;

        //printf("%lld %lld\n",update_version.A,update_version.B);
    }
    
    //verbose 파일 종료
    if (g_b_verbose) {
        fclose(p_file_verbose);
    }

    //초당 throughput 계산
    g_throughput[thread_id] /= g_duration;

    //계산 후 A,B 값의 합 저장
    //printf("thread %d after   A:%lld  B:%lld\n",thread_id,update_version.argument.A,update_version.argument.B);
    g_sum_after_array[thread_id] = update_version.argument.A + update_version.argument.B;

    return NULL;
}
