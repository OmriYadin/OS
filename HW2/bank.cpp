#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "account.h"
#include "rd_wr.h"
#include <map>
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
void* print_screen(void* args);
void* bank_fees(void* args);
void lock(pthread_mutex_t* lock);
void lock_init(pthread_mutex_t* lock);
void lock_destroy(pthread_mutex_t* lock);
void unlock(pthread_mutex_t* lock);


// struct to provide arguments for threads
struct thread_args {
	fstream* file;
	int atm_id;
	pthread_mutex_t* amount_lock;
	pthread_mutex_t* log_lock;
	pthread_mutex_t* trans_lock;
	fstream* log;
	Rd_wr* bank_rd_wr;
	map<int, Account>* accounts;
	bool* is_finished;
	int* bank_amount;
};



int main(int argc, char *argv[]){
	// one thread for each ATM
	pthread_t* atm_threads = new pthread_t[argc-1];
	// one args struct for each thread
	thread_args* args = new thread_args[argc-1];
	pthread_t screen;
	pthread_t fees;
	thread_args screen_args;
	thread_args fees_args;
	// mutex to protect the amount the bank has
	pthread_mutex_t amount_lock;
	// mutex to protect the log file
	pthread_mutex_t log_lock;
	// mutex to prevent deadlock on transfer operations
	// in case of T:1->2 at the same time as T:2->1
	pthread_mutex_t trans_lock;
	fstream log;
	Rd_wr bank_rd_wr;
	map<int, Account> accounts;
	bool is_finished;
	int bank_amount = 0;
	is_finished = false;
	log.open("log.txt", ios_base::out);
	lock_init(&log_lock);
	lock_init(&amount_lock);
	lock_init(&trans_lock);
	if(argc < 2){
		cerr << "Bank error: illegal arguments" << endl;
		return -1;
	}
	fstream *files = new fstream[argc-1];
	int index;
	for(index = 0; index < argc-1; index++){
		files[index].open(argv[index+1], ios_base::in);
		if(!files[index].is_open()){
			cerr << "Bank error: illegal arguments" << endl;
			break;
		}
		// args preparation for each ATM
		args[index].file = &files[index];
		args[index].atm_id = index+1;
		args[index].amount_lock = &amount_lock;
		args[index].log_lock = &log_lock;
		args[index].trans_lock = &trans_lock;
		args[index].log = &log;
		args[index].bank_rd_wr = &bank_rd_wr;
		args[index].accounts = &accounts;
		args[index].is_finished = &is_finished;
		args[index].bank_amount = &bank_amount;
		// end of args prepartion
	}
	
	// in case the program can't open one of the files
	// frees memory and dwstroys mutexes 
	if(index != argc-1){
		for(int i = 0; i < index; i++){
			files[index].close();
		}
		delete[] files;
		delete[] atm_threads;
		delete[] args;
		lock_destroy(&log_lock);
		lock_destroy(&amount_lock);
		lock_destroy(&trans_lock);
		log.close();
		exit(1);
	}
	
	// creates ATM threads
	for(int i = 0; i < argc-1; i++){
		if(pthread_create(&(atm_threads[i]), NULL,
				operations, (void*)(&(args[i])))){
			perror("Bank error: pthread_create failed");
			exit(1);
		}
	}
	
	// screen args preparation
	screen_args.amount_lock = &amount_lock;
	screen_args.bank_rd_wr = &bank_rd_wr;
	screen_args.accounts = &accounts;
	screen_args.is_finished = &is_finished;
	screen_args.bank_amount = &bank_amount;
	// end of screen args preparation
	
	// creates screen thread
	if(pthread_create(&screen, NULL, print_screen, (void*)(&screen_args))){
		perror("Bank error: pthread_create failed");
		exit(1);
	}
	
	// fees args preparation
	fees_args.log = &log;
	fees_args.bank_rd_wr = &bank_rd_wr;
	fees_args.accounts = &accounts;
	fees_args.is_finished = &is_finished;
	fees_args.bank_amount = &bank_amount;
	fees_args.amount_lock = &amount_lock;
	fees_args.log_lock = &log_lock;
	// end of fees args preparation
	
	// creates fees thread
	if(pthread_create(&fees, NULL, bank_fees, (void*)(&fees_args))){
		perror("Bank error: pthread_create failed");
		exit(1);
	}
	
	// waits for all the ATMs to end
	for(int i = 0; i < argc-1; i++){
		if(pthread_join(atm_threads[i], NULL)){
			perror("Bank error: pthread_join failed");
			exit(1);
		}
	}
	
	// is_finished state will end the while loop on fees & screen
	is_finished = true;
	
	// waits for fees thread to end
	if(pthread_join(fees, NULL)){
		perror("Bank error: pthread_join failed");
		exit(1);
	}
	
	// waits for screen thread to end
	if(pthread_join(screen, NULL)){
		perror("Bank error: pthread_join failed");
		exit(1);
	}
	
	// memory free and destroy locks
	for(int i = 0; i<argc-1; i++){
		files[i].close();
	}
	delete[] files;
	delete[] atm_threads;
	delete[] args;
	lock_destroy(&log_lock); 
	lock_destroy(&amount_lock); 
	lock_destroy(&trans_lock);
	log.close();
	return 0;
	
}


// function to init locks and provide perror
void lock_init(pthread_mutex_t* lock){
	if(pthread_mutex_init(lock, NULL)){
		perror("Bank error: pthread_mutex_init failed");
		exit(1);
	}
}

// function to lock locks and provide perror
void lock(pthread_mutex_t* lock){
	if(pthread_mutex_lock(lock)){
		perror("Bank error: pthread_mutex_lock failed");
		exit(1);
	}
}

// function to unlock locks and provide perror
void unlock(pthread_mutex_t* lock){
	if(pthread_mutex_unlock(lock)){
		perror("Bank error: pthread_mutex_unlock failed");
		exit(1);
	}
}

// function to destroy locks and provide perror
void lock_destroy(pthread_mutex_t* lock){
	if(pthread_mutex_destroy(lock)){
		perror("Bank error: pthread_mutex_destroy failed");
		exit(1);
	}
}


// function for ATM operations
void* operations(void* args){
	// extract arguments from void*
	int atm_id = ((thread_args*)args)->atm_id;
	fstream* file = ((thread_args*)args)->file;
	map<int, Account>* accounts = ((thread_args*)args)->accounts;
	Rd_wr* bank_rd_wr = ((thread_args*)args)->bank_rd_wr;
	fstream* log = ((thread_args*)args)->log;
	pthread_mutex_t* log_lock = ((thread_args*)args)->log_lock;
	pthread_mutex_t* trans_lock = ((thread_args*)args)->trans_lock;

	int bal;
	int amount;
	int tg_id;
	map<int, Account>::iterator iter;
	map<int, Account>::iterator iter_1;
	map<int, Account>::iterator iter_2;
	char line[MAX_LEN];
	char* save_ptr;
	while(file->getline(line, MAX_LEN)){
		sleep(0.1);
		char* cur_token;
		cur_token = strtok_r(line, " \t\n", &save_ptr);
		int id = atoi(strtok_r(NULL, " \t\n", &save_ptr));
		int pass = atoi(strtok_r(NULL, " \t\n", &save_ptr));
		Account tmp_account = Account(id,0 ,pass);

		
		switch(cur_token[0]){
			
			case 'O':
				bal = atoi(strtok_r(NULL, " \t\n", &save_ptr));
				tmp_account.upd_balance(DEPOSIT, bal);
				bank_rd_wr->wr_entry();
				// in case account with the same id exists
				if(accounts->find(id) != accounts->end()){
					lock(log_lock);
					bank_rd_wr->wr_exit();
					*log << "Error " << atm_id <<
							": Your transaction failed - account with the same id exists" << endl;
					unlock(log_lock);
					sleep(1);
				}
				// in case we need to create new account
				else {
					iter = accounts->insert({id, tmp_account}).first;
					(iter->second).rd_wr.wr_entry();
					bank_rd_wr->wr_exit();
					lock(log_lock);
					*log << atm_id << ": New account id is " << id <<
							" with password " << pass << " and initial balance "
							<< bal << endl;
					unlock(log_lock);
					sleep(1);
					(iter->second).rd_wr.wr_exit();
				}
				break;
				
			case 'D':
				amount = atoi(strtok_r(NULL, " \t\n", &save_ptr));
				bank_rd_wr->rd_entry();
				iter = accounts->find(id);
				// in case account exists
				if(iter != accounts->end()){
					(iter->second).rd_wr.rd_entry();
					// in case the password is true
					if((iter->second).pass_auth(pass)){
						(iter->second).rd_wr.rd_exit();
						(iter->second).rd_wr.wr_entry();
						bank_rd_wr->rd_exit();
						int new_amount = (iter->second).upd_balance(DEPOSIT, amount);
						lock(log_lock);
						*log << atm_id << ": Account " << id << " new balance is " << 
								new_amount << " after " << amount << " $ was deposited\n";
						unlock(log_lock);
						sleep(1);
						(iter->second).rd_wr.wr_exit();
					}
					// in case the password is incorrect
					else {
						bank_rd_wr->rd_exit();
						lock(log_lock);
						*log << "Error " << atm_id <<": Your transaction failed - password for account id "
								<< id << " is incorrect\n";
						unlock(log_lock);
						sleep(1);
						(iter->second).rd_wr.rd_exit();
					}
				}
				// in case account does not exist
				else {
					lock(log_lock);
					bank_rd_wr->rd_exit();
					*log << "Error " << atm_id << ": Your transaction failed - account id " << id <<
							" does not exist\n";
					unlock(log_lock);
					sleep(1);
				}
				break;
				
			case 'W':
				amount = atoi(strtok_r(NULL, " \t\n", &save_ptr));
				bank_rd_wr->rd_entry();
				iter = accounts->find(id);
				// in case the account exists
				if(iter != accounts->end()){
					(iter->second).rd_wr.rd_entry();
					// in case password is correct
					if((iter->second).pass_auth(pass)){
						// in case there is enough money in the account
						if((iter->second).rd_balance() - amount > 0){
							(iter->second).rd_wr.rd_exit();
							(iter->second).rd_wr.wr_entry();
							bank_rd_wr->rd_exit();
							int new_amount = (iter->second).upd_balance(WITHDRAW, amount);
							lock(log_lock);
							*log << atm_id << ": Account " << id << " new balance is " << 
									new_amount << " after " << amount << " $ was withdrew\n";
							unlock(log_lock);
							sleep(1);
							(iter->second).rd_wr.wr_exit();
						}
						// in case there is not enough mony in the account
						else{
							bank_rd_wr->rd_exit();
							lock(log_lock);
							*log << "Error " << atm_id <<": Your transaction failed - account id "
									<< id << " balance is lower than " << amount << "\n";
							unlock(log_lock);
							sleep(1);
							(iter->second).rd_wr.rd_exit();
						}
					}
					// in case the password is incorrect
					else {
						bank_rd_wr->rd_exit();
						lock(log_lock);
						*log << "Error " << atm_id <<": Your transaction failed - password for account id "
								<< id << " is incorrect\n";
						unlock(log_lock);
						sleep(1);
						(iter->second).rd_wr.rd_exit();
					}
				}
				// in case account does not exist
				else {
					lock(log_lock);
					bank_rd_wr->rd_exit();
					*log << "Error " << atm_id << ": Your transaction failed - account id " << id <<
							" does not exist\n";
					unlock(log_lock);
					sleep(1);
				}
				break;
			
			case 'B':
				bank_rd_wr->rd_entry();
				iter = accounts->find(id);
				// in case account exists
				if(iter != accounts->end()){
					(iter->second).rd_wr.rd_entry();
					// in case password is correct
					if((iter->second).pass_auth(pass)){
						bank_rd_wr->rd_exit();
						bal = (iter->second).rd_balance();
						lock(log_lock);
						*log << atm_id << ": Account " << id << " balance is " << bal << "\n";
						unlock(log_lock);
						sleep(1);
						(iter->second).rd_wr.rd_exit();
					}
					// in case password is incorrect
					else {
						bank_rd_wr->rd_exit();
						lock(log_lock);
						*log << "Error " << atm_id <<": Your transaction failed - password for account id "
								<< id << " is incorrect\n";
						unlock(log_lock);
						sleep(1);
						(iter->second).rd_wr.rd_exit();
					}
				}
				// in case account does not exist
				else {
					lock(log_lock);
					bank_rd_wr->rd_exit();
					*log << "Error " << atm_id << ": Your transaction failed - account id " << id <<
							" does not exist\n";
					unlock(log_lock);
					sleep(1);
				}
				break;
				
			case 'Q':
				bank_rd_wr->wr_entry();
				iter = accounts->find(id);
				// in case account exists
				if(iter != accounts->end()){
					(iter->second).rd_wr.rd_entry();
					// in case password is correct
					if((iter->second).pass_auth(pass)){
						bal = (iter->second).rd_balance();
						(iter->second).rd_wr.rd_exit();
						(iter->second).rd_wr.wr_entry();
						(iter->second).rd_wr.wr_exit();
						accounts->erase(iter);
						lock(log_lock);
						*log << atm_id <<": Account " << id << " is now closed. Balance was "
								<< bal << "\n";
						unlock(log_lock);
						bank_rd_wr->wr_exit();
						sleep(1);
					}
					// in case password is incorrect
					else {
						lock(log_lock);
						bank_rd_wr->wr_exit();
						*log << "Error " << atm_id <<": Your transaction failed - password for account id "
								<< id << " is incorrect\n";
						unlock(log_lock);
						sleep(1);
						(iter->second).rd_wr.rd_exit();
					}
				}
				// in case account does not exist
				else {
					lock(log_lock);
					bank_rd_wr->wr_exit();
					*log << "Error " << atm_id << ": Your transaction failed - account id " << id <<
							" does not exist\n";
					unlock(log_lock);
					sleep(1);
				}
				break;
				
				
			case 'T':
				tg_id = atoi(strtok_r(NULL, " \t\n", &save_ptr));
				amount = atoi(strtok_r(NULL, " \t\n", &save_ptr));
				bank_rd_wr->rd_entry();
				iter_1 = accounts->find(id);
				// in case first account exists
				if(iter_1 != accounts->end()){
					lock(trans_lock);
					(iter_1->second).rd_wr.wr_entry();
					// in case the password of the first account is correct
					if((iter_1->second).pass_auth(pass)){
						Account tmp_account_2 = Account(tg_id, 0, 0);
						iter_2 = accounts->find(tg_id);
						// in case the second accounts exists
						if(iter_2 != accounts->end()){
							(iter_2->second).rd_wr.wr_entry();
							unlock(trans_lock);
							bank_rd_wr->rd_exit();
							bal = (iter_1->second).upd_balance(WITHDRAW, amount);
							// in case the balance is too low
							if(bal == -1){
								lock(log_lock);
								*log << "Error " << atm_id <<": Your transaction failed - account id "
										<< id << " balance is lower than " << amount << "\n";
								unlock(log_lock);
								sleep(1);
								(iter_1->second).rd_wr.wr_exit();
								(iter_2->second).rd_wr.wr_exit();
							}
							// in case the balance is enough
							else {
								int tg_bal = (iter_2->second).upd_balance(DEPOSIT, amount);
								lock(log_lock);
								*log << atm_id << ": Transfer " << amount << " from account " << id <<
										" to account " << tg_id << " new account balance is " << bal <<
										" new target account balance is " << tg_bal << "\n";
								unlock(log_lock);
								sleep(1);
								(iter_1->second).rd_wr.wr_exit();
								(iter_2->second).rd_wr.wr_exit();
							}
						}
						// in case the second account does not exist
						else {
							lock(log_lock);
							unlock(trans_lock);
							bank_rd_wr->rd_exit();
							*log << "Error " << atm_id << ": Your transaction failed - account id " << tg_id <<
									" does not exist\n";
							unlock(log_lock);
							sleep(1);
							(iter_1->second).rd_wr.wr_exit();
						}
					}
					// in case the password of the first account is incorrect
					else {
						lock(log_lock);
						unlock(trans_lock);
						bank_rd_wr->rd_exit();
						*log << "Error " << atm_id <<": Your transaction failed - password for account id "
								<< id << " is incorrect\n";
						unlock(log_lock);
						sleep(1);
						(iter_1->second).rd_wr.wr_exit();
					}
				}
				// in case the first account does not exist
				else {
					lock(log_lock);
					bank_rd_wr->rd_exit();
					*log << "Error " << atm_id << ": Your transaction failed - account id " << id <<
							" does not exist\n";
					unlock(log_lock);
					sleep(1);
				}
				break;
		}
	}
	pthread_exit(NULL);
}


// function in order to collect bank fees
void* bank_fees(void* args){
	// extracts arguments
	map<int, Account>* accounts = ((thread_args*)args)->accounts;
	bool* is_finished = ((thread_args*)args)->is_finished;
	Rd_wr* bank_rd_wr = ((thread_args*)args)->bank_rd_wr;
	pthread_mutex_t* amount_lock = ((thread_args*)args)->amount_lock;
	int* bank_amount = ((thread_args*)args)->bank_amount;
	fstream* log = ((thread_args*)args)->log;
	pthread_mutex_t* log_lock = ((thread_args*)args)->log_lock;
	
	map<int ,Account>::iterator iter;
	int cur_amount;
	// while one of the ATMs is still working
	while(!(*is_finished)){
		sleep(3);
		bank_rd_wr->rd_entry();
		iter = accounts->begin();
		if(accounts->empty()){
			bank_rd_wr->rd_exit();
			continue;
		}
		int cur_fee = (rand()%5) + 1;
		for(; iter != accounts->end(); iter++){
			(iter->second).rd_wr.wr_entry();
			cur_amount = (iter->second).upd_balance(FEE, cur_fee);
			lock(amount_lock);
			*bank_amount += cur_amount;
			unlock(amount_lock);
			lock(log_lock);
			*log << "Bank: commissions of " << cur_fee << " % were charges, the bank gained "
					<< cur_amount << " $ from account " << (iter->second).get_id() << "\n";
			unlock(log_lock);
			(iter->second).rd_wr.wr_exit();
		}
		bank_rd_wr->rd_exit();
	}
	pthread_exit(NULL);
}


// function in order to print accounts snapshot to screen
void* print_screen(void* args){
	// extracts arguments
	map<int, Account>* accounts = ((thread_args*)args)->accounts;
	bool* is_finished = ((thread_args*)args)->is_finished;
	Rd_wr* bank_rd_wr = ((thread_args*)args)->bank_rd_wr;
	pthread_mutex_t* amount_lock =  ((thread_args*)args)->amount_lock;
	int* bank_amount =  ((thread_args*)args)->bank_amount;
	
	map<int, Account>::iterator iter = accounts->begin();
	
	// while one of the ATMs is still working
	while(!(*is_finished)){
		sleep(0.5);
		printf("\033[2J");
		printf("\033[1;H");
		cout << "Current Bank Status" << endl;
		bank_rd_wr->rd_entry();
		iter = accounts->begin();
		if(accounts->empty()){
			bank_rd_wr->rd_exit();
			continue;
		}
		for(; iter != accounts->end(); iter++){
			(iter->second).rd_wr.rd_entry();
			int cur_bal = (iter->second).rd_balance();
			int cur_pass = (iter->second).get_pass();
			int cur_id = (iter->second).get_id();
			cout << "Account " << cur_id << ": Balance - " << cur_bal <<
					" $, Acccount Password - " << cur_pass;
			cout << endl;
			(iter->second).rd_wr.rd_exit();
		}
		
		lock(amount_lock);
		cout << "The Bank has " << *bank_amount << " $" << endl;
		unlock(amount_lock);
		bank_rd_wr->rd_exit();
	}
	
	pthread_exit(NULL);
}
