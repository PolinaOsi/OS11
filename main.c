#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#define MUTEX_COUNT 3
#define SUCCESS 0
#define ERROR 1
#define SECOND 1
#define FIRST_MUTEX 1
#define SECOND_MUTEX 2
#define FIRST_THREAD 1
#define SECOND_THREAD 2

void destroyMutexes(int count, pthread_mutex_t* mutexes){
    for(int i = 0; i < count; ++i){
        errno = pthread_mutex_destroy(&mutexes[i]);
	if(errno != SUCCESS){
            perror("Destoying mutex error");
	}
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
    return SUCCESS;
}

void Print(int thread_number, pthread_mutex_t* mutexes){
    int cur_mutex = FIRST_MUTEX;
    if(thread_number == SECOND_THREAD){
	errno = pthread_mutex_lock(&mutexes[SECOND_MUTEX]);
    	if(errno != SUCCESS){
            perror("Mutex lock error");
	    return;
    	}
	cur_mutex = SECOND_MUTEX;
    }
    for(int i = 0; i < 10; ++i){
    	int next_mutex = (cur_mutex + 2) % 3;
	errno = pthread_mutex_lock(&mutexes[next_mutex]);
    	if(errno != SUCCESS){
            perror("Mutex lock error");
		return;
    	}
        printf("Thread № %d: %d\n", thread_number, i);
	errno = pthread_mutex_unlock(&mutexes[cur_mutex]);
    	if(errno != SUCCESS){
            perror("Mutex unlock error");
	    return;
    	}
	cur_mutex = next_mutex;
    }
    errno = pthread_mutex_unlock(&mutexes[cur_mutex]);
    if(errno != SUCCESS){
        perror("Mutex unlock error");
        return;
    }
}

void* secondPrint(void* param){
    pthread_mutex_t* mutexes = (pthread_mutex_t*)param;
    Print(SECOND_THREAD, mutexes);
    return NULL;
}

int main(int argc, char **argv){
    pthread_t thread;
    pthread_mutex_t mutexes[MUTEX_COUNT];
    if(initMutexes(mutexes) != SUCCESS){
    	exit(EXIT_FAILURE);
    };
    errno = pthread_mutex_lock(&mutexes[FIRST_MUTEX]);
    if(errno != SUCCESS){
	perror("Mutex lock error");
    	destroyMutexes(MUTEX_COUNT, mutexes);
    	exit(EXIT_FAILURE); 
    }
	
    errno = pthread_create(&thread, NULL, secondPrint, mutexes);
    if(errno != SUCCESS){
	 perror("Creating thread error");
	 destroyMutexes(MUTEX_COUNT, mutexes);
   	 exit(EXIT_FAILURE); 
    }

    sleep(SECOND);

    Print(FIRST_THREAD, mutexes);

    errno = pthread_join(thread,NULL);
    if(errno != SUCCESS){
	 perror("Thread join error");
   	 destroyMutexes(MUTEX_COUNT, mutexes);
   	 exit(EXIT_FAILURE); 
    }

    destroyMutexes(MUTEX_COUNT, mutexes);
    exit(0);
}
