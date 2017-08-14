/*
@FILE:		/src/main.cpp
@DATE:		2016/10/29
@AUTHOR:	서치원(suchiwon@hanyang.ac.kr)
@BRIEF:		메인 함수부
*/
#include "global.cpp"
#include "mvcc.cpp"
#include "garbage_collector.cpp"
#include <exception>
#include <time.h>

int test;
/**
메인 함수
인자:
	argc: 가변 인수의 갯수. 3 또는 4이어야함
	argv[]: 가변 인수 배열. argv[1] = 스레드 갯수/ argv[2] = 실행 시간(초)/ argv[3] = verbose 형태
*/
int main(int argc, char* argv[]) {

    //가변 인수의 갯수가 틀리면 실행 금지
    if (!(argc == 3 || argc == 4)) {
        printf("please execute by \n bin/project2 [number of threads] [duration] or\n bin/project2 [number of threads] [duraiton] verbose\n");

        return 0;
    }

    try {
        g_thread_num = atoi(argv[1]);
        g_duration = atoi(argv[2]);

        if (argc == 4 && strcmp(argv[3],"verbose") == 0) {
            g_b_verbose = true;
        }
    } catch (bad_exception &e) {
        puts("please obey argument's type");
        return 0;
    }

    //전역 변수와 락 변수 초기화
    Init();
    LockInit();
    FilterLockInit(g_thread_num);

    //계산 스레드와 가비지 콜렉터 번호 저장 변수
    pthread_t threads[g_thread_num];
    pthread_t garbage_collector[GARBAGE_COLLECTOR_NUM];

    //스레드 실행시 스레드 번호를 정확히 전달하기 위한 변수
    int idx_saver[g_thread_num];

    //스레드의 갯수만큼 함수 실행
    for (int i = 0; i < g_thread_num; i++) {
        idx_saver[i] = i;

        if (pthread_create(&threads[i], 0, ThreadFuncMvcc, &idx_saver[i]) < 0) {
            printf("pthread_create_error\n");
            return 0;
        }
    }

    //가비지 콜렉터 갯수만큼 함수 실행
    if (pthread_create(&garbage_collector[0], 0, ThreadFuncGarbageCollector, &idx_saver[0]) < 0) {
        printf("garbage_collector_create_error\n");
        return 0;
    }
 
    //현재 시점에서 타이머 시작
    clock_t start_clock = clock();
    clock_t verbose_clock = start_clock;

    //지정된 실행 시간 종료까지 타이머 루프
    while (((clock() - start_clock))/CLOCKS_PER_SEC < g_duration) {
	//0.1초 마다 verbose 파일 입력 플래그 생성
        if (((clock() - verbose_clock) * 10)/CLOCKS_PER_SEC >= 1) {
            
            verbose_clock = clock();

            for (int i = 0; i < g_thread_num; i++) {
                g_verbose_input_flag[i] = true;
            }
        }
    }

    //각 스레드에게 신호를 보낼 종료 플래그 저장
    g_duration_end = true;

    //스레드의 종료 기다림
    for (int i = 0; i < g_thread_num; i++) {
        pthread_join(threads[i],NULL);
    }

    for (int i = 0; i < GARBAGE_COLLECTOR_NUM; i++) {
        pthread_join(garbage_collector[i],NULL);
    }

    long sum_before = 0;
    long sum_after = 0;

    //각 스레드의 계산 전, 후 합 저장 값 합산
    for (int i = 0; i < g_thread_num; i++) {
        sum_before += g_sum_before_array[i];
        sum_after += g_sum_after_array[i];

        //printf("%ld %ld\n",g_sum_before_array[i],g_sum_after_array[i]);
        //printf("%d\n",GetListSize(g_mvcc_thread_list_head_arr[i]));
    }

    //계산 전, 후의 합 출력
    printf("SUM OF THREAD BEFORE UPDATE:%ld\n",sum_before);
    printf("SUM OF THREAD AFTER UPDATE:%ld\n",sum_after);
    
    long throughput = 0;
    long fairness_temp = 0;
    double fairness = 0;

    //각 스레드의 throughput 저장
    for (int i = 0; i < g_thread_num; i++) {
        throughput += g_throughput[i];
        fairness_temp += g_throughput[i] * g_throughput[i];
    }

    //throughput과 fairness 출력
    printf("throughput of all threads:%ld\n",throughput);

    fairness = (double)(throughput * throughput)/(g_thread_num * fairness_temp);
    
    printf("fairness of algorithm:%f\n",fairness);

    return 0;
}

