// signals.cpp
// contains signal handler funtions
// contains the function/s that set the signal handlers

/*******************************************/
/* Name: handler_cntlc
   Synopsis: handle the Control-C */
#include "signals.h"
extern int cur_pid;


void ctrl_c_smash_handler(int sig_num){
	sigset_t mask_set;
	sigset_t old_set;
	sigfillset(&mask_set);
	sigprocmask(SIG_SETMASK, &mask_set, &old_set);
	cout << "smash: caught ctrl-C" << endl;
	sigprocmask(SIG_SETMASK, &mask_set, &old_set);
}

void ctrl_c_fg_handler(int sig_num){
	sigset_t mask_set;
	sigset_t old_set;
	sigfillset(&mask_set);
	//sigprocmask(SIG_SETMASK, &mask_set, &old_set);
	cout << "smash: caught ctrl-C" << endl;
	kill(cur_pid, SIGKILL);
	cout << "smash: process " << cur_pid << " was killed" <<endl;
	//sigprocmask(SIG_SETMASK, &mask_set, &old_set);

}


void ctrl_z_smash_handler(int sig_num){
	sigset_t mask_set;
	sigset_t old_set;
	sigfillset(&mask_set);
	sigprocmask(SIG_SETMASK, &mask_set, &old_set);
	cout << "smash: caught ctrl-Z" << endl;
	sigprocmask(SIG_SETMASK, &mask_set, &old_set);
}

void ctrl_z_fg_handler(int sig_num){
	sigset_t mask_set;
	sigset_t old_set;
	sigfillset(&mask_set);
	sigprocmask(SIG_SETMASK, &mask_set, &old_set);
	cout << "smash: caught ctrl-Z" << endl;
	kill(cur_pid, SIGSTOP);
	cout << "smash: process " << cur_pid << " was stopped" <<endl;
	sigprocmask(SIG_SETMASK, &mask_set, &old_set);
}
