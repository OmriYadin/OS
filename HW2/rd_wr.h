#ifndef _RD_WR_H
#define _RD_WR_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

class Rd_wr {
	pthread_mutex_t rd_lock;
	pthread_mutex_t wr_lock;
	pthread_mutex_t del_lock;
	int readers;
	
	public:
	Rd_wr();
	~Rd_wr();
	void rd_entry();
	void rd_exit();
	void wr_entry();
	void wr_exit();
	void lock_del();
	void unlock_del();
};


#endif