#ifndef _RD_WR_H
#define _RD_WR_H

#include <stdio.h>
#include <pthread.h>

class rd_wr {
	pthread_mutex_t rd_lock;
	pthread_mutex_t wr_lock;
	pthread_mutex_t del_lock;
	int readers;
	
	public:
	rd_wr();
	~rd_wr();
	rd_entry();
	rd_exit();
	wr_entry();
	wr_exit();
};