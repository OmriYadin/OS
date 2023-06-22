#include "account.h"

enum OPS {
	DEPOSIT = 1,
	WITHDRAW = 2,
	FEE = 3
};

// Account constructor
Account::Account(int id, int balance, int password){
	this->id = id;
	this->balance = balance;
	this->password = password;
	this->rd_wr = Rd_wr();
}

// operator< for comparison
bool Account::operator<(const Account& account) const{
	return (this->id < account.id);
}

// Account destructor
Account::~Account(){}

// operator= for assignment
Account& Account::operator=(const Account& account){
	this->id = account.id;
	this->balance = account.balance;
	this->password = account.password;
	return *this;
}

// operator== for comparison
bool Account::operator==(const Account& account) const{
	return (this->id == account.id);
}

// password authentication
bool Account::pass_auth(int password){
	return ((this->password) == password);
}

// reads account balance
int Account::rd_balance(){
	int tmp_balance;
	tmp_balance = this->balance;
	return tmp_balance;
}

// prints account data
void Account::print_acc(){
	rd_wr.rd_entry();
	cout << "Account " << this->id << ": - " << this->balance << 
			" $, Account Password - " << this->password << endl;
	rd_wr.rd_exit();
}

// updates account balance
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

// gets account id
int Account::get_id(){
	return this->id;
}

// gets account password
int Account::get_pass(){
	return this->password;
}

