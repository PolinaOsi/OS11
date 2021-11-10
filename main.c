#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#define MUTEX_COUNT 3

void destroyMutexes(int count, pthread_mutex_t* mutexes){
    for(int i = 0; i < count; ++i){
        if(pthread_mutex_destroy(&mutexes[i]) < 0){
            perror("Destoying mutex error");
            exit(EXIT_FAILURE);
        }
    }
}

void atExit(char* str, pthread_mutex_t* mutexes){
    destroyMutexes(MUTEX_COUNT, mutexes);
    perror(str);
    exit(EXIT_FAILURE); 
}

void lockMutex(int num, pthread_mutex_t* mutexes){
    if(pthread_mutex_lock(&mutexes[num])){
         atExit("Mutex lock error", mutexes);
    }
}

void unlockMutex(int num, pthread_mutex_t* mutexes){
    if(pthread_mutex_unlock(&mutexes[num])){ 
        atExit("Mutex unlock error", mutexes);
    }
}

void initMutexes(pthread_mutex_t* mutexes){
    pthread_mutexattr_t mattr;
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
            destroyMutexes(i, mutexes);
            perror("Mutex initilization error");
            exit(EXIT_FAILURE);
        }
    }
}

void Print(int num, pthread_mutex_t* mutexes){
    if(num == 2){
    	lockMutex(2, mutexes);
    }
    for(int i = 0; i < 10; ++i){
	lockMutex((num + 2) % 3, mutexes);
        if(printf("Thread %d: %d\n", num, i) < 0){
            atExit("1st thread printing error", mutexes);
        }
        unlockMutex(num, mutexes);
        lockMutex((num + 1) % 3, mutexes);
        unlockMutex((num + 2) % 3, mutexes);
        lockMutex(num, mutexes);
        unlockMutex((num + 1) % 3, mutexes);
    }
    unlockMutex(num, mutexes);
}

void* secondPrint(void* param){
    pthread_mutex_t* mutexes = (pthread_mutex_t*)param;
    Print(2, mutexes);
    return NULL;
}

int main(int argc, char **argv){
    pthread_t thread;
    pthread_mutex_t mutexes[MUTEX_COUNT];
    initMutexes(mutexes);
    lockMutex(1, mutexes);

    if (pthread_create(&thread, NULL, secondPrint, mutexes)){
        atExit("Creating thread error", mutexes);
    }

    if(sleep(1)){
        atExit("Sleep error", mutexes);
    }

    Print(1, mutexes);

    if (pthread_join(thread,NULL)){
        atExit("Thread join error", mutexes);
    }

    destroyMutexes(MUTEX_COUNT, mutexes);
    exit(0);
}
