#ifndef _ACCOUNT_H
#define _ACCOUNT_H

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include "rd_wr.h"

using namespace std;

class Account {
	int balance;
	int id;
	int password;
	
	public:
	Rd_wr rd_wr;
	Account(int id, int balance, int password);
	~Account();
	bool pass_auth(int password);
	bool operator<(const Account& account) const;
	bool operator==(const Account& account) const;
	Account& operator=(const Account& account);
	int get_id();
	int get_pass();
	int rd_balance();
	void print_acc();
	int upd_balance(int op, int amount);
};

#endif
