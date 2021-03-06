#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

int timeout;

int g_counter = 0;
int g_writer_counter = 0;
int g_reader_counter = 0;

int readAcquires = 0;
int readReleases = 0;
bool b_writer = false;

void writelock()
{
    while (!(__sync_bool_compare_and_swap(&b_writer,false,true))) {
    }
    //b_writer = true;

    while (readAcquires != readReleases) {
    }
}

void writeunlock()
{
    b_writer = false;
}

void readlock()
{
    while (b_writer) {
    }

    __sync_fetch_and_add(&readAcquires,1);
}

void readunlock()
{
    __sync_fetch_and_add(&readReleases,1);
}

void* writer(void* arg)
{
    int i = *((int*) arg);
    int counter = 0; 

    while (!timeout) {
        writelock();

        g_counter++;
        g_writer_counter++;
        counter++;
        usleep(100);

        writeunlock();
    }
    printf("writer %d wrote %d times!\n", i, counter);
    printf("g_writer_counter:%d\n",g_writer_counter);

	return NULL;
}

void* reader(void* arg)
{
    int i = *((int*) arg);
    int counter = 0; 

    while (!timeout) {
        readlock();

        counter++;
        g_counter++;
        g_reader_counter++;
        usleep(100);

        readunlock();
    }
    printf("reader %d read %d times!\n", i, counter);
	
    printf("g_reader_counter:%d\n",g_reader_counter);
	return NULL;
}

int main()
{
    //init
    int num_writer = 2;
    int num_reader = 10;
    printf("Test rwlock concurrency(%d writer, %d reader) started.\n", num_writer, num_reader);

    //test
    timeout = 0;
    pthread_t writer_thread[num_writer];
    int writer_index[num_writer];
    for (int i = 0; i < num_writer; i++) {
        pthread_create(&writer_thread[i], NULL, writer, (void*) &(writer_index[i]=i+1));
    }
    pthread_t reader_thread[num_reader];
    int reader_index[num_reader];
    for (int i = 0; i < num_reader; i++) {
        pthread_create(&reader_thread[i], NULL, reader, (void*) &(reader_index[i]=i+1));
    }

    usleep(5000000);
    timeout = 1;

    for (int i = 0; i < num_writer; i++) {
        pthread_join(writer_thread[i], NULL);
    }
    for (int i = 0; i < num_reader; i++) {
        pthread_join(reader_thread[i], NULL);
    }

    //assert
    printf("Test rwlock concurrency(%d writer, %d reader) passed.\n", num_writer, num_reader);
    printf("g_counter = %d\n",g_counter);
}
