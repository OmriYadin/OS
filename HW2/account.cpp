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
	this->rd_wr.lock_del();
}

bool Account::operator<(const Account& account) const{
	return (this->id < account.id);
}

Account::~Account(){}


bool Account::operator==(const Account& account) const{
	return (this->id == account.id);
}


bool Account::operator==(const int id) const{
	return (this->id == id);
}

bool Account::pass_auth(int password) const{
	return ((this->password) == password);
}

void Account::open_locks() const{
	this->rd_wr.unlock_del();
}

int Account::rd_balance() const{
	int tmp_balance;
	tmp_balance = this->balance;
	return tmp_balance;
}


void Account::print_acc() const{
	rd_wr.rd_entry();
	cout << "Account " << this->id << ": - " << this->balance << 
			" $, Account Password - " << this->password << endl;
	rd_wr.rd_exit();
}


int Account::upd_balance(int op, int amount) const{
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

int Account::get_id() const{
	return this->id;
}

int Account::get_pass() const{
	return this->password;
}

