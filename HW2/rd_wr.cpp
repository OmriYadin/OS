#include "rd_wr.h"

using namespace std;

// readers writers as we learned in class
// we also added delete lock to prevent starvations and 
// to prevent errors when we delete accounts

// readers writers constructor
Rd_wr::Rd_wr(){
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

// readers writers destructor
Rd_wr::~Rd_wr(){
	if(pthread_mutex_lock(&rd_lock)){
		perror("Bank error: pthread_mutex_lock failed");
		exit(1);
	}
	
	if(pthread_mutex_unlock(&rd_lock)){
		perror("Bank error: pthread_mutex_unlock failed");
		exit(1);
	}
	
	if(pthread_mutex_destroy(&rd_lock)){
		perror("Bank error: pthread_mutex_destroy failed");
		exit(1);
	}

	if(pthread_mutex_destroy(&wr_lock)){
		perror("Bank error: pthread_mutex_destroy failed");
		exit(1);
	}

	if(pthread_mutex_destroy(&del_lock)){
		perror("Bank error: pthread_mutex_destroy failed");
		exit(1);
	}
	

	
}


// readers writers read entry
void Rd_wr::rd_entry(){
	if(pthread_mutex_lock(&del_lock)){
		perror("Bank error: pthread_mutex_lock failed");
		exit(1);
	}
	if(pthread_mutex_lock(&rd_lock)){
		perror("Bank error: pthread_mutex_lock failed");
		exit(1);
	}
	this->readers += 1;
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


// readers writers read exit
void Rd_wr::rd_exit(){
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


// readers writers write entry
void Rd_wr::wr_entry(){
	if(pthread_mutex_lock(&del_lock)){
		perror("Bank error: pthread_mutex_lock failed");
		exit(1);
	}
	if(pthread_mutex_lock(&wr_lock)){
		perror("Bank error: pthread_mutex_lock failed");
		exit(1);
	}
	//cout << "entry_write" << endl;
	if(pthread_mutex_unlock(&del_lock)){
		perror("Bank error: pthread_mutex_unlock failed");
		exit(1);
	}
}


// readers writers write exit
void Rd_wr::wr_exit(){
	if(pthread_mutex_unlock(&wr_lock)){
		perror("Bank error: pthread_mutex_unlock failed");
		exit(1);
	}
}

