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
	this->rd_count = 0;
	this->rd_lock = Sem(0);
	this->wr_lock = Sem(0);
}

bool operator<(const Account& account){
	return (this->id < account->id);
}

bool operator==(const Account& account){
	return (this->id == account->id)
}

bool pass_auth(int password){
	return ((this->password) == password);
}

void Account::open_locks(){
	this->wr_lock.up();
	this->rd_lock.up();
}

int Account::rd_balance(){
	int tmp_balance;
	this->rd_lock.down();
	this->rd_count += 1;
	if((this->rd_count) == 1){
		this->wr_lock.down();
	}
	this->rd_lock.up();
	tmp_balance = this->balance;
	this->rd_lock.down();
	this->rd_count -= 1;
	if((this->rd_count) == 0){
		this->wr_lock.up();
	}
	this->rd_lock.up();
	return tmp_balance;
}


void Account::print_acc(){
	this->rd_lock.down();
	this->rd_count += 1;
	if((this->rd_count) == 1){
		this->wr_lock.down();
	}
	this->rd_lock.up();
	cout << "Account " << this->id << ": - " << this->balance << 
			" $, Account Password - " << this->password << endl;
	this->rd_lock.down();
	this->rd_count -= 1;
	if((this->rd_count) == 0){
		this->wr_lock.up();
	}
	this->rd_lock.up();
}


int Account::upd_balance(int op, int amount){
	this->wr_lock.down();
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
	this->wr_lock.up();
	return upd_amount;
}
