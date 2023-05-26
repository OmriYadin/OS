#include "sem.h"

Sem::Sem(int cnt){
	if(pthread_mutex_init(&lock, NULL)){
		perror("Bank error: pthread_mutex_init failed");
		exit(1);
	}
	if(pthread_mutex_init(&sync, NULL)){
		perror("Bank error: pthread_mutex_init failed");
		exit(1);
	}
	this->counter = cnt;
	if(pthread_mutex_lock(&sync)){
		perror("Bank error: pthread_mutex_lock failed");
		exit(1);
	}
}

void Sem::up(){
	if(pthread_mutex_lock(&lock)){
		perror("Bank error: pthread_mutex_lock failed");
		exit(1);
	}
	counter++;
	if(pthread_mutex_unlock(&sync)){
		perror("Bank error: pthread_mutex_unlock failed");
		exit(1);
	}
	if(pthread_mutex_unlock(&lock)){
		perror("Bank error: pthread_mutex_unlock failed");
		exit(1);
	}
}

void Sem::down(){
	if(pthread_mutex_lock(&lock)){
		perror("Bank error: pthread_mutex_lock failed");
		exit(1);
	}
	while(!counter){
		if(pthread_mutex_unlock(&lock)){
			perror("Bank error: pthread_mutex_unlock failed");
			exit(1);
		}
		if(pthread_mutex_lock(&sync)){
			perror("Bank error: pthread_mutex_lock failed");
			exit(1);
		}
		if(pthread_mutex_lock(&lock)){
			perror("Bank error: pthread_mutex_lock failed");
			exit(1);
		}
	}
	counter--;
	if(pthread_mutex_unlock(&lock)){
		perror("Bank error: pthread_mutex_unlock failed");
		exit(1);
	}
}