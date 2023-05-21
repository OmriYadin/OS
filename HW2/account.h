#ifndef _ACCOUNT_H
#define _ACCOUNT_H

#include <stdio.h>
#include <iostream>
#include "sem.h"

using namespace std;

class Account {
	int balance;
	int id;
	int password;
	int rd_count;
	Sem rd_lock;
	Sem wr_lock;
	
	public:
	Account(int id, int balance, int password);
	bool pass_auth(int password);
	bool operator<(const Account& account);
	bool operator==(const Account& account);
	int rd_balance();
	void print_acc();
	int upd_balance(int op, int amount);
};
