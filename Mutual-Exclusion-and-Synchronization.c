#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
//Number of slots in buffer
#define SlotsNumber 10
//Controls access to critical region 1
sem_t Mutex;
//Counts empty buffer slots SlotsNumber
sem_t Empty;
//Counts full buffer slots 0
sem_t Full;
sem_t Counter;
sem_t Read;
int Buffer_Counter = 0,i=0,size=0,Stack[SlotsNumber],End=SlotsNumber-1,Start=0;
//Pushing elements into the queue if it's not full
void StackPush(unsigned char Input)
{
    while (size != SlotsNumber)
    {
        End = (End + 1) % SlotsNumber;
        size++;
        Stack[End] = Input;
    }
}

void Stack_Pop()
{
    int Output=0;
    while (size > 0)
    {
        Output = Stack[Start];
        Start = (Start + 1) % SlotsNumber;
        size--;
    }
}
//Random time for sleep
void Random_Sleep(int Value)
{
    sleep(rand()%Value);
}
//count independent incoming messages in a system
void* Count_Thread(int Thread_Number)
{
    while(1)
    {
        Random_Sleep(5);
        printf("\nCounter thread %d: recieved a message\n",Thread_Number);
        printf("\nCounter thread %d: waiting to write\n",Thread_Number);
        sem_wait(&Counter);
        Buffer_Counter=Buffer_Counter+1;
        printf("\nCounter thread %d: now adding to counter, counter value= %d\n",Thread_Number,Buffer_Counter);
        sem_post(&Counter);
    }
}
//gets the count of threads at time intervals of size t1
//then resets the counter to 0 then places this value in a buffer of size b
void* Monitor_Thread()
{
    while(1)
    {
        Random_Sleep(5);
        printf("\nMonitor thread: waiting to read counter\n");
        sem_wait(&Counter);
        printf("\nMonitor thread: reading a count value of %d\n",Buffer_Counter);
        sem_post(&Counter);
        while(size == SlotsNumber)
        {
            printf("\nMonitor thread: Buffer full!\n");
            Random_Sleep(5);
        }
        sem_wait(&Full);
        sem_wait(&Read);
        StackPush(Buffer_Counter);
        printf("\nMonitor thread: writing to buffer at position %d\n",End);
        Buffer_Counter = 0;
        sem_post(&Read);
        sem_post(&Empty);
    }
}
//reads the values from the buffer.
void* Collect_Thread()
{
    while(1)
    {
        Random_Sleep(5);
        if(size > 0)
        {
            sem_wait(&Empty);
            sem_wait(&Read);
            printf("\nCollector thread: Reading from buffer at position %d\n",Start);
            Stack_Pop();
            sem_post(&Read);
            sem_post(&Full);
        }
        else
        {
            printf("\nCollector thread: nothing is in the buffer!\n");
            Random_Sleep(5);
        }
    }
}

void main()
{
//initialize semaphores
    sem_init(&Read, 0, 1);
    sem_init(&Counter, 0, 1);
    sem_init(&Mutex, 0, 1);
    sem_init(&Full, 0, 10);
    sem_init(&Empty, 0, 0);
    pthread_t Thread_Monitor,Thread_Collect;
    pthread_t Array_Threads[6];
    i=0;
    while(i<6)
    {
        int* Thread_ID = (int*) malloc(sizeof(int));
        Thread_ID = i;
        pthread_create(&Array_Threads[i], NULL, Count_Thread, Thread_ID);
        i++;
    }
    pthread_create(&Thread_Monitor, NULL, Monitor_Thread, NULL);
    pthread_create(&Thread_Collect, NULL, Collect_Thread, NULL);
    i=0;
    while(i<6)
    {
        pthread_join(Array_Threads[i], NULL);
        i++;
    }
    pthread_join(Thread_Monitor,NULL);
    pthread_join(Thread_Collect,NULL);
    sem_destroy(&Empty);
    sem_destroy(&Read);
    sem_destroy(&Counter);
    sem_destroy(&Full);
}

