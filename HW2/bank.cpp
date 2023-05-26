#include <stdio.h>
#include <iostream>
#include "account.h"
#include <pthread.h>
#include <fstream>

#define MAX_LEN 80 

enum OPS {
	DEPOSIT = 1,
	WITHDRAW = 2,
	FEE = 3
};

using namespace std;



fstream log;
pthread_mutex_t log_lock;
pthread_mutex_t enter_lock;

int main(int argc, char *argv[]){
	log.open("log.txt", ios_base::out);
	if(pthread_mutex_init(&log_lock, NULL)){
		perror("Bank error: pthread_mutex_init failed");
		exit(1);
	}
	if(pthread_mutex_init(&enter_lock, NULL)){
		perror("Bank error: pthread_mutex_init failed");
		exit(1);
	}
	if(argc < 2){
		cerr << "Bank error: illegal arguments" << endl;
		
	}
	fstream *files = new fstream[argc-1];
	bool is_open = true;
	int index;
	for(index = 0; index < argc-1; index++){
		files[index].open(argv[index+1], ios_base::in);
		if(!files[index].is_open()){
			cerr << "Bank error: illegal arguments" << endl;
			break;
		}
	}
	if(index != argc-1){
		for(int i = 0; i<index; i++){
			files[index].close();
		}
		delete[] files;
		pthread_mutex_destroy(&log_lock);
		log.close();
		exit(1);
	}
	set<Account> accounts;
	
}
	

void operations(fstream file, int atm_id){
	char line[MAX_LEN];
	while(!file.eof()){
		file.getline(line, MAX_LEN, '\n');
		char* cur_token;
		cur_token = strtok(line, " \t");
		
		switch(cur_token){
			
			case "O":
				int id = atoi(strtok(NULL, " /t"));
				int pass = atoi(strtok(NULL, " /t"));
				int bal = atoi(strtok(NULL, " /t"));
				Account tmp_account = Account(id, bal, pass);
				pthread_mutex_lock(&enter_lock);
				if(accounts->count(tmp_account)){
					pthread_mutex_lock(&log_lock);
					log << "Error " << atm_id <<
							": Your transaction failed - account with the same id exists\n";
					pthread_mutex_unlock(&log_lock);
				}
				else {
					insert(tmp_account);
					((Account)accounts->find(tmp_account))->open_locks();
					pthread_mutex_lock(&log_lock);
					log << atm_id << ": New account id is " << id <<
							" with password " << pass << " and initial balance "
							<< bal << "\n";
					pthread_mutex_unlock(&log_lock);
				}
				pthread_mutex_unlock(&enter_lock);
				break;
				
			case "D":
				
				
				
			case "W":
				
			
			case "B":
				
				
			case "Q":
				
				
			case "T":
			
		}
		
	}
}