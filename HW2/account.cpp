#include "account.h"

enum OPS {
	DEPOSIT = 1,
	WITHDRAW = 2,
	FEE = 3
};

Account::Account(int id, int balance, int password){
	this->id = id;
	this->balance = balance;
	this->password = password;
	this->rd_wr = Rd_wr();
	//this->rd_wr.lock_del();
}

bool Account::operator<(const Account& account) const{
	return (this->id < account.id);
}

Account::~Account(){}

Account& Account::operator=(const Account& account){
	this->id = account.id;
	this->balance = account.balance;
	this->password = account.password;
	return *this;
}

bool Account::operator==(const Account& account) const{
	return (this->id == account.id);
}


bool Account::pass_auth(int password){
	return ((this->password) == password);
}

void Account::open_locks(){
	this->rd_wr.unlock_del();
}

int Account::rd_balance(){
	int tmp_balance;
	tmp_balance = this->balance;
	return tmp_balance;
}


void Account::print_acc(){
	rd_wr.rd_entry();
	cout << "Account " << this->id << ": - " << this->balance << 
			" $, Account Password - " << this->password << endl;
	rd_wr.rd_exit();
}


int Account::upd_balance(int op, int amount){
	int upd_amount = -1;
	switch(op){
		case DEPOSIT:
			this->balance += amount;
			upd_amount = this->balance;
			break;
		
		case WITHDRAW:
			if((this->balance) >= amount){
				this->balance -= amount;
				upd_amount = this->balance;
			}
			break;
			
		case FEE:
			upd_amount = (int)((this->balance)*((double)(amount)/100));
			this->balance -= upd_amount;
			break;
	}
	return upd_amount;
}

int Account::get_id(){
	return this->id;
}

int Account::get_pass(){
	return this->password;
}

