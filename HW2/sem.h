#ifndef _SEM_H
#define _SEM_H

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>


class Sem {
	pthread_mutex_t lock;
	pthread_mutex_t sync;
	int counter;
	
	public:
	Sem(int cnt);
	~Sem();
	void up();
	void down();
} ;

