/*
@FILE:		/src/main.cpp
@DATE:		2016/10/29
@AUTHOR:	��ġ��(suchiwon@hanyang.ac.kr)
@BRIEF:		���� �Լ���
*/
#include "global.cpp"
#include "mvcc.cpp"
#include "garbage_collector.cpp"
#include <exception>
#include <time.h>

int test;
/**
���� �Լ�
����:
	argc: ���� �μ��� ����. 3 �Ǵ� 4�̾����
	argv[]: ���� �μ� �迭. argv[1] = ������ ����/ argv[2] = ���� �ð�(��)/ argv[3] = verbose ����
*/
int main(int argc, char* argv[]) {

    //���� �μ��� ������ Ʋ���� ���� ����
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

    //���� ������ �� ���� �ʱ�ȭ
    Init();
    LockInit();
    FilterLockInit(g_thread_num);

    //��� ������� ������ �ݷ��� ��ȣ ���� ����
    pthread_t threads[g_thread_num];
    pthread_t garbage_collector[GARBAGE_COLLECTOR_NUM];

    //������ ����� ������ ��ȣ�� ��Ȯ�� �����ϱ� ���� ����
    int idx_saver[g_thread_num];

    //�������� ������ŭ �Լ� ����
    for (int i = 0; i < g_thread_num; i++) {
        idx_saver[i] = i;

        if (pthread_create(&threads[i], 0, ThreadFuncMvcc, &idx_saver[i]) < 0) {
            printf("pthread_create_error\n");
            return 0;
        }
    }

    //������ �ݷ��� ������ŭ �Լ� ����
    if (pthread_create(&garbage_collector[0], 0, ThreadFuncGarbageCollector, &idx_saver[0]) < 0) {
        printf("garbage_collector_create_error\n");
        return 0;
    }
 
    //���� �������� Ÿ�̸� ����
    clock_t start_clock = clock();
    clock_t verbose_clock = start_clock;

    //������ ���� �ð� ������� Ÿ�̸� ����
    while (((clock() - start_clock))/CLOCKS_PER_SEC < g_duration) {
	//0.1�� ���� verbose ���� �Է� �÷��� ����
        if (((clock() - verbose_clock) * 10)/CLOCKS_PER_SEC >= 1) {
            
            verbose_clock = clock();

            for (int i = 0; i < g_thread_num; i++) {
                g_verbose_input_flag[i] = true;
            }
        }
    }

    //�� �����忡�� ��ȣ�� ���� ���� �÷��� ����
    g_duration_end = true;

    //�������� ���� ��ٸ�
    for (int i = 0; i < g_thread_num; i++) {
        pthread_join(threads[i],NULL);
    }

    for (int i = 0; i < GARBAGE_COLLECTOR_NUM; i++) {
        pthread_join(garbage_collector[i],NULL);
    }

    long sum_before = 0;
    long sum_after = 0;

    //�� �������� ��� ��, �� �� ���� �� �ջ�
    for (int i = 0; i < g_thread_num; i++) {
        sum_before += g_sum_before_array[i];
        sum_after += g_sum_after_array[i];

        //printf("%ld %ld\n",g_sum_before_array[i],g_sum_after_array[i]);
        //printf("%d\n",GetListSize(g_mvcc_thread_list_head_arr[i]));
    }

    //��� ��, ���� �� ���
    printf("SUM OF THREAD BEFORE UPDATE:%ld\n",sum_before);
    printf("SUM OF THREAD AFTER UPDATE:%ld\n",sum_after);
    
    long throughput = 0;
    long fairness_temp = 0;
    double fairness = 0;

    //�� �������� throughput ����
    for (int i = 0; i < g_thread_num; i++) {
        throughput += g_throughput[i];
        fairness_temp += g_throughput[i] * g_throughput[i];
    }

    //throughput�� fairness ���
    printf("throughput of all threads:%ld\n",throughput);

    fairness = (double)(throughput * throughput)/(g_thread_num * fairness_temp);
    
    printf("fairness of algorithm:%f\n",fairness);

    return 0;
}

