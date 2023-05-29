#include <stdio.h>
#include <iostream>
#include "account.h"
#include "rd_wr.h"
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
Rd_wr bank_rd_wr;
int bank_amount = 0;

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
	

void operations(fstream file, int atm_id, set<Account>* accounts){
	char line[MAX_LEN];
	while(!file.eof()){
		file.getline(line, MAX_LEN, '\n');
		char* cur_token;
		cur_token = strtok(line, " \t");
		int id = atoi(strtok(NULL, " /t"));
		int pass = atoi(strtok(NULL, " /t"));
		
		switch(cur_token){
			
			case "O":
				int bal = atoi(strtok(NULL, " /t"));
				Account tmp_account = Account(id, bal, pass);
				bank_rd_wr.wr_entry();
				if(accounts->count(tmp_account)){
					pthread_mutex_lock(&log_lock);///////////////////////////
					bank_rd_wr.wr_exit();
					log << "Error " << atm_id <<
							": Your transaction failed - account with the same id exists\n";
					pthread_mutex_unlock(&log_lock);
				}
				else {
					accounts->insert(tmp_account);
					pthread_mutex_lock(&log_lock);///////////////////////////
					bank_rd_wr.wr_exit();
					log << atm_id << ": New account id is " << id <<
							" with password " << pass << " and initial balance "
							<< bal << "\n";
					pthread_mutex_unlock(&log_lock);///////////////////////
					((Account*)accounts->find(tmp_account))->open_locks();
				}
				break;
				
			case "D":
				int amount = atoi(strtok(NULL, " /t"));
				bank_rd_wr.rd_entry();
				set<Account>::iterator iter = accounts->find(id);
				if(iter != accounts->end()){
					((Account*)iter)->rd_wr.rd_entry();
					if((Account*)iter->pass_auth(pass)){
						(Account*)iter->rd_wr.rd_exit();
						(Account*)iter->rd_wr.wr_entry();
						bank_rd_wr.rd_exit();
						int new_amount = (Account*)iter->upd_balance(DEPOSIT, amount);
						pthread_mutex_lock(&log_lock); //////////////////////////////
						(Account*)iter->rd_wr.wr_exit();
						log << atm_id << ": Account " << id << " new balance is " << 
								new_amount << " after " << amount << " $ was deposited\n";
						pthread_mutex_unlock(&log_lock);///////////////////////
					}
					else {
						bank_rd_wr.rd_exit();
						sleep(1);
						pthread_mutex_lock(&log_lock); //////////////////////////////
						(Account*)iter->rd_wr.rd_exit();
						log << "Error " << atm_id <<": Your transaction failed - password for account id "
								<< id << " is incorrect\n";
						pthread_mutex_unlock(&log_lock);///////////////////////
					}
				}
				else {
					sleep(1);
					pthread_mutex_lock(&log_lock); ///////////////////////////
					bank_rd_wr.rd_exit();
					log << "Error " << atm_id << ": Your transaction failed - account id " << id <<
							" does not exist\n";
					pthread_mutex_unlock(&log_lock);///////////////////////
				}
				
			case "W":
				int amount = atoi(strtok(NULL, " /t"));
				bank_rd_wr.rd_entry();
				set<Account>::iterator iter = accounts->find(id);
				if(iter != accounts->end()){
					((Account*)iter)->rd_wr.rd_entry();
					if((Account*)iter->pass_auth(pass)){
						if((Account*)iter->rd_balance() - amount > 0){
							(Account*)iter->rd_wr.rd_exit();
							(Account*)iter->rd_wr.wr_entry();
							bank_rd_wr.rd_exit();
							int new_amount = (Account*)iter->upd_balance(DEPOSIT, amount);
							pthread_mutex_lock(&log_lock); //////////////////////////////
							(Account*)iter->rd_wr.wr_exit();
							log << atm_id << ": Account " << id << " new balance is " << 
									new_amount << " after " << amount << " $ was withdrew\n";
							pthread_mutex_unlock(&log_lock);///////////////////////
						}
						else{
							bank_rd_wr.rd_exit();
							sleep(1);
							pthread_mutex_lock(&log_lock); //////////////////////////////
							(Account*)iter->rd_wr.rd_exit();
							log << "Error " << atm_id <<": Your transaction failed - account id "
									<< id << " balance is lower than " << amount << "\n";
							pthread_mutex_unlock(&log_lock);///////////////////////
						}
					}
					else {
						bank_rd_wr.rd_exit();
						sleep(1);
						pthread_mutex_lock(&log_lock); //////////////////////////////
						(Account*)iter->rd_wr.rd_exit();
						log << "Error " << atm_id <<": Your transaction failed - password for account id "
								<< id << " is incorrect\n";
						pthread_mutex_unlock(&log_lock);///////////////////////
					}
				}
				else {
					sleep(1);
					pthread_mutex_lock(&log_lock); ///////////////////////////
					bank_rd_wr.rd_exit();
					log << "Error " << atm_id << ": Your transaction failed - account id " << id <<
							" does not exist\n";
					pthread_mutex_unlock(&log_lock);///////////////////////
				}
			
			case "B":
				bank_rd_wr.rd_entry();
				set<Account>::iterator iter = accounts->find(id);
				if(iter != accounts->end()){
					((Account*)iter)->rd_wr.rd_entry();
					if((Account*)iter->pass_auth(pass)){
						bank_rd_wr.rd_exit();
						int bal = ((Account*)iter)->rd_balance();
						sleep(1);
						pthread_mutex_lock(&log_lock); //////////////////////////////
						(Account*)iter->rd_wr.rd_exit();
						log << atm_id << ": Account " << id << " balance is " << bal << "\n";
						pthread_mutex_unlock(&log_lock);///////////////////////
					} 
					else {
						bank_rd_wr.rd_exit();
						sleep(1);
						pthread_mutex_lock(&log_lock); //////////////////////////////
						(Account*)iter->rd_wr.rd_exit();
						log << "Error " << atm_id <<": Your transaction failed - password for account id "
								<< id << " is incorrect\n";
						pthread_mutex_unlock(&log_lock);///////////////////////
					}
				}
				else {
					sleep(1);
					pthread_mutex_lock(&log_lock); ///////////////////////////
					bank_rd_wr.rd_exit();
					log << "Error " << atm_id << ": Your transaction failed - account id " << id <<
							" does not exist\n";
					pthread_mutex_unlock(&log_lock);///////////////////////
				}
			
				
			case "Q":
				
				
			case "T":
			
		}
		
	}
}