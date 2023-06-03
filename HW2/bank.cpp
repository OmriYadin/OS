#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "account.h"
#include "rd_wr.h"
#include <set>
#include <pthread.h>
#include <fstream>

#define MAX_LEN 80 

enum OPS {
	DEPOSIT = 1,
	WITHDRAW = 2,
	FEE = 3
};

using namespace std;

void* operations(void* args);
void* print_screen(void*);
void* bank_fees(void*);
void lock(pthread_mutex_t* lock);
void unlock(pthread_mutex_t* lock);

struct thread_args {
	fstream* file;
	int atm_id;
};


pthread_mutex_t amount_lock;
pthread_mutex_t log_lock;
fstream log;
Rd_wr bank_rd_wr;
set<Account> accounts;
bool is_finished;
int bank_amount = 0;



int main(int argc, char *argv[]){
	pthread_t* atm_threads = new pthread_t[argc-1];
	thread_args* args = new thread_args[argc-1];
	pthread_t screen;
	pthread_t fees;
	is_finished = false;
	log.open("log.txt", ios_base::out);
	if(pthread_mutex_init(&log_lock, NULL)){
		perror("Bank error: pthread_mutex_init failed");
		exit(1);
	}
	if(pthread_mutex_init(&amount_lock, NULL)){
		perror("Bank error: pthread_mutex_init failed");
		exit(1);
	}
	if(argc < 2){
		cerr << "Bank error: illegal arguments" << endl;
		return -1;
	}
	fstream *files = new fstream[argc-1];
	//bool is_open = true;
	int index;
	for(index = 0; index < argc-1; index++){
		files[index].open(argv[index+1], ios_base::in);
		if(!files[index].is_open()){
			cerr << "Bank error: illegal arguments" << endl;
			break;
		}
		args[index].file = &files[index];
		args[index].atm_id = index+1;
	}
	
	if(index != argc-1){
		for(int i = 0; i<index; i++){
			files[index].close();
		}
		delete[] files;
		delete[] atm_threads;
		delete[] args;
		pthread_mutex_destroy(&log_lock); ///////////////
		pthread_mutex_destroy(&amount_lock); ////////////
		log.close();
		exit(1);
	}
	
	for(int i = 0; i < argc-1; i++){
		if(!(pthread_create(&(atm_threads[i]), NULL,
				operations, (void*)(&args[i])))){
			perror("Bank error: pthread_create failed");
			exit(1);
		}
	}
	
	if(!(pthread_create(&screen, NULL, print_screen, NULL))){
		perror("Bank error: pthread_create failed");
		exit(1);
	}
	
	if(!(pthread_create(&fees, NULL, bank_fees, NULL))){
		perror("Bank error: pthread_create failed");
		exit(1);
	}
	
	for(int i = 0; i < argc-1; i++){
		if(!(pthread_join(atm_threads[i], NULL))){
			perror("Bank error: pthread_join failed");
			exit(1);
		}
	}
	
	is_finished = true;
	
	if(!(pthread_join(fees, NULL))){
		perror("Bank error: pthread_join failed");
		exit(1);
	}
	
	if(!(pthread_join(screen, NULL))){
		perror("Bank error: pthread_join failed");
		exit(1);
	}
	
	for(int i = 0; i<argc-1; i++){
		files[i].close();
	}
	delete[] files;
	delete[] atm_threads;
	delete[] args;
	pthread_mutex_destroy(&log_lock); ///////////////
	pthread_mutex_destroy(&amount_lock); ////////////
	log.close();
	return 0;
	
}


void lock(pthread_mutex_t* lock){
	if(pthread_mutex_lock(lock)){
		perror("Bank error: lock failed");
		exit(1);
	}
}


void unlock(pthread_mutex_t* lock){
	if(pthread_mutex_unlock(lock)){
		perror("Bank error: unlock failed");
		exit(1);
	}
}


void* operations(void* args){
	int atm_id = ((thread_args*)args)->atm_id;
	fstream* file = ((thread_args*)args)->file;
	int bal;
	int amount;
	int tg_id;
	set<Account>::iterator iter;
	set<Account>::iterator iter_1;
	set<Account>::iterator iter_2;
	Account tmp_account(0,0,0);
	char line[MAX_LEN];
	while(!file->eof()){
		sleep(0.1);
		file->getline(line, MAX_LEN, '\n');
		char* cur_token;
		cur_token = strtok(line, " \t");
		int id = atoi(strtok(NULL, " \t"));
		int pass = atoi(strtok(NULL, " \t"));
		
		switch(cur_token[0]){
			
			case 'O':
				bal = atoi(strtok(NULL, " \t"));
				tmp_account = Account(id, bal, pass);
				bank_rd_wr.wr_entry();
				if(accounts.count(tmp_account)){
					lock(&log_lock);///////////////////////////
					bank_rd_wr.wr_exit();
					log << "Error " << atm_id <<
							": Your transaction failed - account with the same id exists\n";
					unlock(&log_lock);
					sleep(1);
				}
				else {
					iter = accounts.insert(tmp_account);
					lock(&log_lock);///////////////////////////
					bank_rd_wr.wr_exit();
					log << atm_id << ": New account id is " << id <<
							" with password " << pass << " and initial balance "
							<< bal << "\n";
					unlock(&log_lock);///////////////////////
					sleep(1);
					((Account)*iter).open_locks();
				}
				break;
				
			case 'D':
				amount = atoi(strtok(NULL, " \t"));
				bank_rd_wr.rd_entry();
				tmp_account = Account(id, 0, 0);
				iter = accounts.find(tmp_account);
				if(iter != accounts.end()){
					((Account)*iter).rd_wr.rd_entry();
					if(((Account)*iter).pass_auth(pass)){
						((Account)*iter).rd_wr.rd_exit();
						((Account)*iter).rd_wr.wr_entry();
						bank_rd_wr.rd_exit();
						int new_amount = ((Account)*iter).upd_balance(DEPOSIT, amount);
						lock(&log_lock); //////////////////////////////
						log << atm_id << ": Account " << id << " new balance is " << 
								new_amount << " after " << amount << " $ was deposited\n";
						unlock(&log_lock);///////////////////////
						sleep(1);
						((Account)*iter).rd_wr.wr_exit();
					}
					else {
						bank_rd_wr.rd_exit();
						lock(&log_lock); //////////////////////////////
						log << "Error " << atm_id <<": Your transaction failed - password for account id "
								<< id << " is incorrect\n";
						unlock(&log_lock);///////////////////////
						sleep(1);
						((Account)*iter).rd_wr.rd_exit();
					}
				}
				else {
					lock(&log_lock); ///////////////////////////
					bank_rd_wr.rd_exit();
					log << "Error " << atm_id << ": Your transaction failed - account id " << id <<
							" does not exist\n";
					unlock(&log_lock);///////////////////////
					sleep(1);
				}
				break;
				
			case 'W':
				amount = atoi(strtok(NULL, " \t"));
				bank_rd_wr.rd_entry();
				tmp_account = Account(id, 0, 0);
				iter = accounts.find(tmp_account);
				if(iter != accounts.end()){
					((Account)*iter).rd_wr.rd_entry();
					if(((Account)*iter).pass_auth(pass)){
						if(((Account)*iter).rd_balance() - amount > 0){
							((Account)*iter).rd_wr.rd_exit();
							((Account)*iter).rd_wr.wr_entry();
							bank_rd_wr.rd_exit();
							int new_amount = ((Account)*iter).upd_balance(DEPOSIT, amount);
							lock(&log_lock); //////////////////////////////
							log << atm_id << ": Account " << id << " new balance is " << 
									new_amount << " after " << amount << " $ was withdrew\n";
							unlock(&log_lock);///////////////////////
							sleep(1);
							((Account)*iter).rd_wr.wr_exit();
						}
						else{
							bank_rd_wr.rd_exit();
							lock(&log_lock); //////////////////////////////
							log << "Error " << atm_id <<": Your transaction failed - account id "
									<< id << " balance is lower than " << amount << "\n";
							unlock(&log_lock);///////////////////////
							sleep(1);
							((Account)*iter).rd_wr.rd_exit();
						}
					}
					else {
						bank_rd_wr.rd_exit();
						lock(&log_lock); //////////////////////////////
						log << "Error " << atm_id <<": Your transaction failed - password for account id "
								<< id << " is incorrect\n";
						unlock(&log_lock);///////////////////////
						sleep(1);
						((Account)*iter).rd_wr.rd_exit();
					}
				}
				else {
					lock(&log_lock); ///////////////////////////
					bank_rd_wr.rd_exit();
					log << "Error " << atm_id << ": Your transaction failed - account id " << id <<
							" does not exist\n";
					unlock(&log_lock);///////////////////////
					sleep(1);
				}
				break;
			
			case 'B':
				bank_rd_wr.rd_entry();
				tmp_account = Account(id, 0, 0);
				iter = accounts.find(tmp_account);
				if(iter != accounts.end()){
					((Account)*iter).rd_wr.rd_entry();
					if(((Account)*iter).pass_auth(pass)){
						bank_rd_wr.rd_exit();
						bal = ((Account)*iter).rd_balance();
						lock(&log_lock); /////////////////////////////
						log << atm_id << ": Account " << id << " balance is " << bal << "\n";
						unlock(&log_lock);///////////////////////
						sleep(1);
						((Account)*iter).rd_wr.rd_exit();
					} 
					else {
						bank_rd_wr.rd_exit();
						lock(&log_lock); //////////////////////////////
						log << "Error " << atm_id <<": Your transaction failed - password for account id "
								<< id << " is incorrect\n";
						unlock(&log_lock);///////////////////////
						sleep(1);
						((Account)*iter).rd_wr.rd_exit();
					}
				}
				else {
					lock(&log_lock); ///////////////////////////
					bank_rd_wr.rd_exit();
					log << "Error " << atm_id << ": Your transaction failed - account id " << id <<
							" does not exist\n";
					unlock(&log_lock);///////////////////////
					sleep(1);
				}
				break;
				
			case 'Q':
				bank_rd_wr.wr_entry();
				tmp_account = Account(id, 0, 0);
				iter = accounts.find(tmp_account);
				if(iter != accounts.end()){
					((Account)*iter).rd_wr.rd_entry();
					if(((Account)*iter).pass_auth(pass)){
						bal = ((Account)*iter).rd_balance();
						((Account)*iter).rd_wr.rd_exit();
						accounts.erase(iter);
						lock(&log_lock); //////////////////////////////
						log << atm_id <<": Account " << id << " is now closed. Balance was "
								<< bal << "\n";
						unlock(&log_lock);///////////////////////
						bank_rd_wr.wr_exit();
						sleep(1);
					}
					else {
						bank_rd_wr.wr_exit();
						lock(&log_lock); //////////////////////////////
						log << "Error " << atm_id <<": Your transaction failed - password for account id "
								<< id << " is incorrect\n";
						unlock(&log_lock);///////////////////////
						sleep(1);
						((Account)*iter).rd_wr.rd_exit();
					}
				}
				else {
					lock(&log_lock); ///////////////////////////
					bank_rd_wr.wr_exit();
					log << "Error " << atm_id << ": Your transaction failed - account id " << id <<
							" does not exist\n";
					unlock(&log_lock);///////////////////////
					sleep(1);
				}
				break;
				
				
			case 'T':
				tg_id = atoi(strtok(NULL, " \t"));
				amount = atoi(strtok(NULL, " \t"));
				bank_rd_wr.rd_entry();
				tmp_account = Account(id, 0, 0);
				iter_1 = accounts.find(tmp_account);
				if(iter_1 != accounts.end()){
					((Account)*iter_1).rd_wr.wr_entry();
					if(((Account)*iter_1).pass_auth(pass)){
						tmp_account = Account(tg_id, 0, 0);
						iter_2 = accounts.find(tmp_account);
						if(iter_2 != accounts.end()){
							((Account)*iter_2).rd_wr.wr_entry();
							bank_rd_wr.rd_exit();
							bal = ((Account)*iter_1).upd_balance(WITHDRAW, amount);
							if(bal == -1){
								lock(&log_lock); //////////////////////////////
								log << "Error " << atm_id <<": Your transaction failed - account id "
										<< id << " balance is lower than " << amount << "\n";
								unlock(&log_lock);///////////////////////
								sleep(1);
								((Account)*iter_1).rd_wr.wr_exit();
								((Account)*iter_2).rd_wr.wr_exit();
							}
							else {
								int tg_bal = ((Account)*iter_2).upd_balance(DEPOSIT, amount);
								lock(&log_lock); //////////////////////////////
								log << atm_id << ": Transfer " << amount << " from account " << id <<
										" to account " << tg_id << " new account balance is " << bal <<
										" new target account balance is " << tg_bal << "\n";
								unlock(&log_lock);///////////////////////
								sleep(1);
								((Account)*iter_1).rd_wr.wr_exit();
								((Account)*iter_2).rd_wr.wr_exit();
							}
						}
						else {
							lock(&log_lock); ///////////////////////////
							bank_rd_wr.rd_exit();
							log << "Error " << atm_id << ": Your transaction failed - account id " << tg_id <<
									" does not exist\n";
							unlock(&log_lock);///////////////////////
							sleep(1);
							((Account)*iter_1).rd_wr.wr_exit();
						}
					}
					else {
						bank_rd_wr.rd_exit();
						lock(&log_lock); //////////////////////////////
						log << "Error " << atm_id <<": Your transaction failed - password for account id "
								<< id << " is incorrect\n";
						unlock(&log_lock);///////////////////////
						sleep(1);
						((Account)*iter_1).rd_wr.rd_exit();
					}
				}
				else {
					lock(&log_lock); ///////////////////////////
					bank_rd_wr.rd_exit();
					log << "Error " << atm_id << ": Your transaction failed - account id " << tg_id <<
							" does not exist\n";
					unlock(&log_lock);///////////////////////
					sleep(1);
				}
				break;

		}
	}
	pthread_exit(NULL);
}


void* bank_fees(void*){
	set<Account>::iterator iter;
	int cur_amount;
	while(!is_finished){
		sleep(3);
		bank_rd_wr.rd_entry();
		iter = accounts.begin();
		int cur_fee = (rand()%5) + 1;
		for(; iter != accounts.end(); iter++){
			((Account)*iter).rd_wr.wr_entry();
			cur_amount = ((Account)*iter).upd_balance(FEE, cur_fee);
			lock(&amount_lock);/////////////////////////////////////////
			bank_amount += cur_amount;
			unlock(&amount_lock);/////////////////////////////////////////
			lock(&log_lock); ///////////////////////////////////////
			log << "Bank: commissions of " << cur_fee << " % were charges, the bank gained "
					<< cur_amount << " $ from account " << ((Account)*iter).get_id() << "\n";
			unlock(&log_lock); /////////////////////////////////////
			((Account)*iter).rd_wr.wr_exit();
		}
		bank_rd_wr.rd_exit();
	}
	pthread_exit(NULL);
}


void* print_screen(void*){
	set<Account>::iterator iter;
	while(!is_finished){
		sleep(0.5);
		printf("\033[2J");
		printf("\033[1;H");
		cout << "Current Bank Status" << endl;
		bank_rd_wr.rd_entry();
		iter = accounts.begin();
		for(; iter != accounts.end(); iter++){
			((Account)*iter).rd_wr.rd_entry();
			int cur_bal = ((Account)*iter).rd_balance();
			int cur_pass = ((Account)*iter).get_pass();
			int cur_id = ((Account)*iter).get_id();
			cout << "Account " << cur_id << ": Balance - " << cur_bal <<
					" $, Acccount Password - " << cur_pass << endl;
			((Account)*iter).rd_wr.rd_exit();
		}
		
		lock(&amount_lock);/////////////////////////////////////////
		cout << "The Bank has " << bank_amount << " $" << endl;
		unlock(&amount_lock);/////////////////////////////////////////
		bank_rd_wr.rd_exit();
	}
	pthread_exit(NULL);
}
