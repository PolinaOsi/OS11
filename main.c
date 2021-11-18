#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#define MUTEX_COUNT 3
#define SUCCESS 0
#define ERROR 1

void destroyMutexes(int count, pthread_mutex_t* mutexes){
    for(int i = 0; i < count; ++i){
        errno = pthread_mutex_destroy(&mutexes[i]);
	if(errno != SUCCESS){
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
    errno = pthread_mutex_lock(&mutexes[num]);
    if(errno != SUCCESS){
         atExit("Mutex lock error", mutexes);
    }
}

void unlockMutex(int num, pthread_mutex_t* mutexes){
    errno = pthread_mutex_unlock(&mutexes[num]);
    if(errno != SUCCESS){
        atExit("Mutex unlock error", mutexes);
    }
}

int initMutexes(pthread_mutex_t* mutexes){
    pthread_mutexattr_t mattr;
    errno = pthread_mutexattr_init(&mattr);
    if(errno != SUCCESS){
	perror("Attributes initilization error\n");
	return ERROR;
    }
	
    errno = pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_ERRORCHECK);
    if(errno != SUCCESS){
	perror("Attributes creation error\n");
	return ERROR;
    }
	
    for(int i = 0; i < MUTEX_COUNT; ++i){
        errno = pthread_mutex_init(&mutexes[i], &mattr);
	if(errno != SUCCESS){
            destroyMutexes(i, mutexes);
            perror("Mutex initilization error");
            return ERROR;
        }
    }
}

void Print(int num, pthread_mutex_t* mutexes){
    if(num == 2){
    	lockMutex(2, mutexes);
    }
    for(int i = 0; i < 10; ++i){
	lockMutex((num + 2) % 3, mutexes);
        printf("Thread № %d: %d\n", num, i);
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
    
    errno = pthread_create(&thread, NULL, secondPrint, mutexes);
    if(errno != SUCCESS){
        atExit("Creating thread error", mutexes);
    }

    sleep(1);

    Print(1, mutexes);

    errno = pthread_join(thread,NULL);
    if(errno != SUCCESS){
        atExit("Thread join error", mutexes);
    }

    destroyMutexes(MUTEX_COUNT, mutexes);
    exit(0);
}
