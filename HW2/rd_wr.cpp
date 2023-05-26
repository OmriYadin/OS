#include "rd_wr.h"



rd_wr::rd_wr(){
	this->readers = 0;
	if(pthread_mutex_init(&rd_lock, NULL)){
		perror("Bank error: pthread_mutex_init failed");
		exit(1);
	}
	if(pthread_mutex_init(&wr_lock, NULL)){
		perror("Bank error: pthread_mutex_init failed");
		exit(1);
	}
	if(pthread_mutex_init(&del_lock, NULL)){
		perror("Bank error: pthread_mutex_init failed");
		exit(1);
	}
}


rd_wr::~rd_wr(){
	if(pthread_mutex_lock(&del_lock)){
		perror("Bank error: pthread_mutex_lock failed");
		exit(1);
	}
	if(pthread_mutex_lock(&wr_lock)){
		perror("Bank error: pthread_mutex_lock failed");
		exit(1);
	}
	if(pthread_mutex_lock(&rd_lock)){
		perror("Bank error: pthread_mutex_lock failed");
		exit(1);
	}
	if(pthread_mutex_destroy(&del_lock)){
		perror("Bank error: pthread_mutex_destroy failed");
		exit(1);
	}
	if(pthread_mutex_destroy(&wr_lock)){
		perror("Bank error: pthread_mutex_destroy failed");
		exit(1);
	}
	if(pthread_mutex_destroy(&rd_lock)){
		perror("Bank error: pthread_mutex_destroy failed");
		exit(1);
	}
	
}


rd_wr::rd_entry(){
	if(pthread_mutex_lock(&del_lock)){
		perror("Bank error: pthread_mutex_lock failed");
		exit(1);
	}
	if(pthread_mutex_lock(&rd_lock)){
		perror("Bank error: pthread_mutex_lock failed");
		exit(1);
	}
	readers += 1;
	if(readers == 1){
		if(pthread_mutex_lock(&wr_lock)){
			perror("Bank error: pthread_mutex_lock failed");
			exit(1);
		}
	}
	if(pthread_mutex_unlock(&rd_lock)){
		perror("Bank error: pthread_mutex_unlock failed");
		exit(1);
	}
	if(pthread_mutex_unlock(&del_lock)){
		perror("Bank error: pthread_mutex_unlock failed");
		exit(1);
	}
}


rd_wr::rd_exit(){
	if(pthread_mutex_lock(&rd_lock)){
		perror("Bank error: pthread_mutex_lock failed");
		exit(1);
	}
	readers -= 1;
	if(readers == 0){
		if(pthread_mutex_unlock(&wr_lock)){
			perror("Bank error: pthread_mutex_unlock failed");
			exit(1);
		}
	}
	if(pthread_mutex_unlock(&rd_lock)){
		perror("Bank error: pthread_mutex_unlock failed");
		exit(1);
	}
}


rd_wr::wr_entry(){
	if(pthread_mutex_lock(&del_lock)){
		perror("Bank error: pthread_mutex_lock failed");
		exit(1);
	}
	if(pthread_mutex_lock(&wr_lock)){
		perror("Bank error: pthread_mutex_lock failed");
		exit(1);
	}
	if(pthread_mutex_unlock(&del_lock)){
		perror("Bank error: pthread_mutex_unlock failed");
		exit(1);
	}
}


rd_wr::wr_exit(){
	if(pthread_mutex_unlock(&wr_lock)){
		perror("Bank error: pthread_mutex_unlock failed");
		exit(1);
	}
}
