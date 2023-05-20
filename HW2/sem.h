#ifndef _SEM_H
#define _SEM_H

#include <stdio.h>
#include <pthread.h>

class Sem {
	pthread_mutex_t lock;
	pthread_mutex_t sync;
	int counter;
	
	public:
	Sem(int cnt);
	void up();
	void down();
} ;

