#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#define MUTEX_COUNT 3

pthread_mutexattr_t mattr;
pthread_mutex_t mutexes[MUTEX_COUNT];

void destroyMutexes(int count){
    for(int i = 0; i < count; ++i){
        if(pthread_mutex_destroy(&mutexes[i]) < 0){
            perror("Destoying mutex error");
            exit(EXIT_FAILURE);
        }
    }
}

void atExit(char* str){
    destroyMutexes(MUTEX_COUNT);
    perror(str);
    exit(EXIT_FAILURE); 
}

void lockMutex(int num){
    if(pthread_mutex_lock(&mutexes[num])){
         atExit("Mutex lock error");
    }
}

void unlockMutex(int num){
    if(pthread_mutex_unlock(&mutexes[num])){ 
        atExit("Mutex unlock error");
    }
}

void initMutexes(){
    if(pthread_mutexattr_init(&mattr)){
	perror("Attributes initilization error\n");
	exit(EXIT_FAILURE);
    }
    if (pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_ERRORCHECK)){
	perror("Attributes creation error\n");
	exit(EXIT_FAILURE);
    }
    for(int i = 0; i < MUTEX_COUNT; ++i){
        if(pthread_mutex_init(&mutexes[i], &mattr)){
            destroyMutexes(i);
            perror("Mutex initilization error");
            exit(EXIT_FAILURE);
        }
    }
}

void* secondPrint(void* param){
    lockMutex(2);
    for(int i = 0; i < 10; ++i){
        lockMutex(1);
        if(printf("Child: %d\n", i) < 0){
            atExit("2nd thread printing error");
        }
        unlockMutex(2);
        lockMutex(0);
        unlockMutex(1);
        lockMutex(2);
        unlockMutex(0);
    }
    unlockMutex(2);
    return NULL;
}
void firstPrint(){
    for(int i = 0; i < 10; ++i){
        if(printf("Parent: %d\n", i) < 0){
            atExit("1st thread printing error");
        }
        lockMutex(0);
        unlockMutex(1);
        lockMutex(2);
        unlockMutex(0);
        lockMutex(1);
        unlockMutex(2);
    }
    unlockMutex(1);
}
int main(int argc, char **argv){
    pthread_t thread;

    initMutexes();
    lockMutex(1);

    if (pthread_create(&thread, NULL, secondPrint, NULL)){
        atExit("Creating thread error");
    }

    if(sleep(1)){
        atExit("Sleep error");
    }

    firstPrint();

    if (pthread_join(thread,NULL)){
        atExit("Thread join error");
    }

    destroyMutexes(MUTEX_COUNT);
    exit(0);
}
