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
pthread_mutex_t amount_lock;

int main(int argc, char *argv[]){
	pthread_mutex_t log_lock;
	fstream log;
	int bank_amount = 0;
	Rd_wr bank_rd_wr;
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
	

void operations(fstream file, int atm_id, set<Account>* accounts, pthread_mutex_t* log_lock, 
		Rd_wr* bank_rd_wr, fstream* log){
	char line[MAX_LEN];
	while(!file.eof()){
		sleep(0.1);
		file.getline(line, MAX_LEN, '\n');
		char* cur_token;
		cur_token = strtok(line, " \t");
		int id = atoi(strtok(NULL, " /t"));
		int pass = atoi(strtok(NULL, " /t"));
		
		switch(cur_token){
			
			case "O":
				int bal = atoi(strtok(NULL, " /t"));
				Account tmp_account = Account(id, bal, pass);
				bank_rd_wr->wr_entry();
				if(accounts->count(tmp_account)){
					pthread_mutex_lock(log_lock);///////////////////////////
					bank_rd_wr->wr_exit();
					log << "Error " << atm_id <<
							": Your transaction failed - account with the same id exists\n";
					pthread_mutex_unlock(log_lock);
					sleep(1);
				}
				else {
					accounts->insert(tmp_account);
					pthread_mutex_lock(log_lock);///////////////////////////
					bank_rd_wr->wr_exit();
					log << atm_id << ": New account id is " << id <<
							" with password " << pass << " and initial balance "
							<< bal << "\n";
					pthread_mutex_unlock(log_lock);///////////////////////
					sleep(1);
					((Account*)accounts->find(tmp_account))->open_locks();
				}
				break;
				
			case "D":
				int amount = atoi(strtok(NULL, " /t"));
				bank_rd_wr->rd_entry();
				set<Account>::iterator iter = accounts->find(id);
				if(iter != accounts->end()){
					((Account*)iter)->rd_wr.rd_entry();
					if((Account*)iter->pass_auth(pass)){
						(Account*)iter->rd_wr.rd_exit();
						(Account*)iter->rd_wr.wr_entry();
						bank_rd_wr.rd_exit();
						int new_amount = (Account*)iter->upd_balance(DEPOSIT, amount);
						pthread_mutex_lock(log_lock); //////////////////////////////
						log << atm_id << ": Account " << id << " new balance is " << 
								new_amount << " after " << amount << " $ was deposited\n";
						pthread_mutex_unlock(log_lock);///////////////////////
						sleep(1);
						(Account*)iter->rd_wr.wr_exit();
					}
					else {
						bank_rd_wr->rd_exit();
						pthread_mutex_lock(log_lock); //////////////////////////////
						log << "Error " << atm_id <<": Your transaction failed - password for account id "
								<< id << " is incorrect\n";
						pthread_mutex_unlock(log_lock);///////////////////////
						sleep(1);
						(Account*)iter->rd_wr.rd_exit();
					}
				}
				else {
					pthread_mutex_lock(log_lock); ///////////////////////////
					bank_rd_wr->rd_exit();
					log << "Error " << atm_id << ": Your transaction failed - account id " << id <<
							" does not exist\n";
					pthread_mutex_unlock(log_lock);///////////////////////
					sleep(1);
				}
				break;
				
			case "W":
				int amount = atoi(strtok(NULL, " /t"));
				bank_rd_wr->rd_entry();
				set<Account>::iterator iter = accounts->find(id);
				if(iter != accounts->end()){
					((Account*)iter)->rd_wr.rd_entry();
					if((Account*)iter->pass_auth(pass)){
						if((Account*)iter->rd_balance() - amount > 0){
							(Account*)iter->rd_wr.rd_exit();
							(Account*)iter->rd_wr.wr_entry();
							bank_rd_wr->rd_exit();
							int new_amount = (Account*)iter->upd_balance(DEPOSIT, amount);
							pthread_mutex_lock(log_lock); //////////////////////////////
							log << atm_id << ": Account " << id << " new balance is " << 
									new_amount << " after " << amount << " $ was withdrew\n";
							pthread_mutex_unlock(log_lock);///////////////////////
							sleep(1);
							(Account*)iter->rd_wr.wr_exit();
						}
						else{
							bank_rd_wr->rd_exit();
							pthread_mutex_lock(log_lock); //////////////////////////////
							log << "Error " << atm_id <<": Your transaction failed - account id "
									<< id << " balance is lower than " << amount << "\n";
							pthread_mutex_unlock(log_lock);///////////////////////
							sleep(1);
							(Account*)iter->rd_wr.rd_exit();
						}
					}
					else {
						bank_rd_wr->rd_exit();
						pthread_mutex_lock(log_lock); //////////////////////////////
						log << "Error " << atm_id <<": Your transaction failed - password for account id "
								<< id << " is incorrect\n";
						pthread_mutex_unlock(log_lock);///////////////////////
						sleep(1);
						(Account*)iter->rd_wr.rd_exit();
					}
				}
				else {
					pthread_mutex_lock(log_lock); ///////////////////////////
					bank_rd_wr->rd_exit();
					log << "Error " << atm_id << ": Your transaction failed - account id " << id <<
							" does not exist\n";
					pthread_mutex_unlock(log_lock);///////////////////////
					sleep(1);
				}
				break;
			
			case "B":
				bank_rd_wr->rd_entry();
				set<Account>::iterator iter = accounts->find(id);
				if(iter != accounts->end()){
					((Account*)iter)->rd_wr.rd_entry();
					if((Account*)iter->pass_auth(pass)){
						bank_rd_wr->rd_exit();
						int bal = ((Account*)iter)->rd_balance();
						pthread_mutex_lock(log_lock); /////////////////////////////
						log << atm_id << ": Account " << id << " balance is " << bal << "\n";
						pthread_mutex_unlock(log_lock);///////////////////////
						sleep(1);
						(Account*)iter->rd_wr.rd_exit();
					} 
					else {
						bank_rd_wr->rd_exit();
						pthread_mutex_lock(log_lock); //////////////////////////////
						log << "Error " << atm_id <<": Your transaction failed - password for account id "
								<< id << " is incorrect\n";
						pthread_mutex_unlock(log_lock);///////////////////////
						sleep(1);
						(Account*)iter->rd_wr.rd_exit();
					}
				}
				else {
					pthread_mutex_lock(log_lock); ///////////////////////////
					bank_rd_wr->rd_exit();
					log << "Error " << atm_id << ": Your transaction failed - account id " << id <<
							" does not exist\n";
					pthread_mutex_unlock(&log_lock);///////////////////////
					sleep(1);
				}
				break;
				
			case "Q":
				bank_rd_wr->wr_entry();
				set<Account>::iterator iter = accounts->find(id);
				if(iter != accounts->end()){
					((Account*)iter)->rd_wr.rd_entry();
					if((Account*)iter->pass_auth(pass)){
						int bal = (Account*)iter->rd_balance();
						((Account*)iter)->rd_wr.rd_exit();
						accounts->erase(iter);
						pthread_mutex_lock(log_lock); //////////////////////////////
						log << atm_id <<": Account " << id << " is now closed. Balance was " <<
								<< bal << "\n"
						pthread_mutex_unlock(log_lock);///////////////////////
						bank_rd_wr->wr_exit();
						sleep(1);
					}
					else {
						bank_rd_wr->wr_exit();
						pthread_mutex_lock(log_lock); //////////////////////////////
						log << "Error " << atm_id <<": Your transaction failed - password for account id "
								<< id << " is incorrect\n";
						pthread_mutex_unlock(log_lock);///////////////////////
						sleep(1);
						(Account*)iter->rd_wr.rd_exit();
					}
				}
				else {
					pthread_mutex_lock(log_lock); ///////////////////////////
					bank_rd_wr.wr_exit();
					log << "Error " << atm_id << ": Your transaction failed - account id " << id <<
							" does not exist\n";
					pthread_mutex_unlock(log_lock);///////////////////////
					sleep(1);
				}
				break;
				
				
			case "T":
				int tg_id = atoi(strtok(NULL, " /t"));
				int amount = atoi(strtok(NULL, " /t"));
				bank_rd_wr->rd_entry();
				set<Account>::iterator iter_1 = accounts->find(id);
				if(iter_1 != accounts.end()){
					(Account*)iter_1->wr_entry();
					if((Account*)iter_1->auth_pass(pass)){
						set<Account>::iterator iter_2 = accounts->find(tg_id);
						if(iter_2 != accounts.end()){
							(Account*)iter_2->wr_entry();
							bank_rd_wr->rd_exit();
							int bal = (Account*)iter_1->upd_balance(WITHDRAW, amount);
							if(bal == -1){
								pthread_mutex_lock(log_lock); //////////////////////////////
								log << "Error " << atm_id <<": Your transaction failed - account id "
										<< id << " balance is lower than " << amount << "\n";
								pthread_mutex_unlock(log_lock);///////////////////////
								sleep(1);
								(Account*)iter_1->rd_wr.wr_exit();
								(Account*)iter_2->rd_wr.wr_exit();
							}
							else {
								int tg_bal = (Account*)iter_2->upd_balance(DEPOSIT, amount);
								pthread_mutex_lock(log_lock); //////////////////////////////
								log << atm_id << ": Transfer " << amount << " from account " << id <<
										" to account " << tg_id << " new account balance is " << bal <<
										" new target account balance is " << tg_bal << "\n";
								pthread_mutex_unlock(log_lock);///////////////////////
								sleep(1);
								(Account*)iter_1->rd_wr.wr_exit();
								(Account*)iter_2->rd_wr.wr_exit();
							}
						}
						else {
							pthread_mutex_lock(log_lock); ///////////////////////////
							bank_rd_wr->rd_exit();
							log << "Error " << atm_id << ": Your transaction failed - account id " << tg_id <<
									" does not exist\n";
							pthread_mutex_unlock(log_lock);///////////////////////
							sleep(1);
							(Account*)iter_1->rd_wr.wr_exit();
						}
					}
					else {
						bank_rd_wr->rd_exit();
						pthread_mutex_lock(log_lock); //////////////////////////////
						log << "Error " << atm_id <<": Your transaction failed - password for account id "
								<< id << " is incorrect\n";
						pthread_mutex_unlock(log_lock);///////////////////////
						sleep(1);
						(Account*)iter_1->rd_wr.rd_exit();
					}
				}
				else {
					pthread_mutex_lock(log_lock); ///////////////////////////
					bank_rd_wr->rd_exit();
					log << "Error " << atm_id << ": Your transaction failed - account id " << tg_id <<
							" does not exist\n";
					pthread_mutex_unlock(log_lock);///////////////////////
					sleep(1);
				}
				break;
				
		}
	}
}


void bank_fees(int* bank_amount, set<Account>* accounts, bool* is_finished,
		Rd_wr* bank_rd_wr, pthread_mutex_t* log_lock, fstream* log){
	set<Account>::iterator iter;
	while(!(*is_finished)){
		sleep(3);
		bank_rd_wr->rd_entry();
		iter = accounts->begin();
		int cur_fee = (rand()%5) + 1;
		for(; iter != accounts->end(); iter++){
			(Account*)iter->rd_wr.wr_entry();
			cur_amount = (Account*)iter->upd_balance(FEE, cur_fee);
			pthread_mutex_lock(&amount_lock);/////////////////////////////////////////
			*bank_amount += cur_amount;
			pthread_mutex_unlock(&amount_lock);/////////////////////////////////////////
			pthread_mutex_lock(log_lock); ///////////////////////////////////////
			log << "Bank: commissions of " << cur_fee << " % were charges, the bank gained "
					<< cur_amount << " $ from account " << (Account*)iter->get_id() << "\n";
			pthread_mutex_unlock(log_lock); /////////////////////////////////////
			(Account*)iter->rd_wr.wr_exit();
		}
		bank_rd_wr->rd_exit();
	}
	pthread_exit();
}


void print_screen(int* bank_amount, set<Account>* accounts, bool* is_finished, Rd_wr* bank_rd_wr){
	set<Account>::iterator iter;
	while(!(*is_finished)){
		sleep(0.5);
		printf("\033[2J");
		printf("\033[1;H");
		cout << "Current Bank Status" << endl;
		bank_rd_wr->rd_entry();
		iter = accounts->begin();
		for(; iter != accounts->end(); iter++){
			(Account*)iter->rd_wr.rd_entry();
			int cur_bal = (Account*)iter->rd_balance();
			int cur_pass = (Account*)iter->get_pass();
			int cur_id = (Account*)iter->get_id();
			cout << "Account " << cur_id << ": Balance - " << cur_bal <<
					" $, Acccount Password - " << cur_pss << endl;
			(Account*)iter->rd_wr.rd_exit()l
		}
		pthread_mutex_lock(&amount_lock);/////////////////////////////////////////
		cout << "The Bank has " << (*bank_amount) << " $" << endl;
		pthread_mutex_unlock(&amount_lock);/////////////////////////////////////////
		bank_rd_wr->rd_exit();
	}
	pthread_exit();
}
