#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#define MUTEX_COUNT 3
#define SUCCESS 0

void destroyMutexes(int count, pthread_mutex_t* mutexes){
    for(int i = 0; i < count; ++i){
        int err = pthread_mutex_destroy(&mutexes[i]);
	if(err != SUCCESS){
	    errno = err;
            perror("Destoying mutex error");
            exit(EXIT_FAILURE);
        }
    }
}

void atExit(int err, char* str, pthread_mutex_t* mutexes){
    destroyMutexes(MUTEX_COUNT, mutexes);
    errno = err;
    perror(str);
    exit(EXIT_FAILURE); 
}

void lockMutex(int num, pthread_mutex_t* mutexes){
    int err = pthread_mutex_lock(&mutexes[num]);
    if(err != SUCCESS){
         atExit(err, "Mutex lock error", mutexes);
    }
}

void unlockMutex(int num, pthread_mutex_t* mutexes){
    int err = pthread_mutex_unlock(&mutexes[num]);
    if(err != SUCCESS){
        atExit(err, "Mutex unlock error", mutexes);
    }
}

void initMutexes(pthread_mutex_t* mutexes){
    pthread_mutexattr_t mattr;
    int err = pthread_mutexattr_init(&mattr);
    if(err != SUCCESS){
	errno = err;
	perror("Attributes initilization error\n");
	exit(EXIT_FAILURE);
    }
	
    err = pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_ERRORCHECK);
    if(err != SUCCESS){
	errno = err;
	perror("Attributes creation error\n");
	exit(EXIT_FAILURE);
    }
	
    for(int i = 0; i < MUTEX_COUNT; ++i){
        err = pthread_mutex_init(&mutexes[i], &mattr);
	if(err != SUCCESS){
            destroyMutexes(i, mutexes);
	    errno = err;
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
    
    int err = pthread_create(&thread, NULL, secondPrint, mutexes);
    if(err != SUCCESS){
        atExit(err, "Creating thread error", mutexes);
    }

    sleep(1);

    Print(1, mutexes);

     err = pthread_join(thread,NULL);
    if(err != SUCCESS){
        atExit(err, "Thread join error", mutexes);
    }

    destroyMutexes(MUTEX_COUNT, mutexes);
    exit(0);
}
