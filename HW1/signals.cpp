// signals.cpp
// contains signal handler funtions
// contains the function/s that set the signal handlers

/*******************************************/
/* Name: handler_cntlc
   Synopsis: handle the Control-C */
#include "signals.h"
extern int cur_pid;
extern int smash_pid;

/*void ctrl_c_smash_handler(int sig_num){
	sigset_t mask_set;
	sigset_t old_set;
	sigfillset(&mask_set);
	sigprocmask(SIG_SETMASK, &mask_set, &old_set);
	cout << "smash: caught ctrl-C" << endl;
	sigprocmask(SIG_SETMASK, &mask_set, &old_set);
}
*/
void ctrl_c_handler(int sig_num){
	sigset_t mask_set;
	sigset_t old_set;
	sigfillset(&mask_set);
	sigprocmask(SIG_SETMASK, &mask_set, &old_set);
	cout << "smash: caught ctrl-C" << endl;
	if(cur_pid != smash_pid){
		//int pid = getpid();
		kill(cur_pid, SIGKILL);
		cout << "smash: process " << cur_pid << " was killed" << endl;
	}
	sigprocmask(SIG_SETMASK, &mask_set, &old_set);

}

void ctrl_z_handler(int sig_num){
	sigset_t mask_set;
	sigset_t old_set;
	sigfillset(&mask_set);
	sigprocmask(SIG_SETMASK, &mask_set, &old_set);
	cout << "smash: caught ctrl-Z" << endl;
	if(cur_pid != smash_pid){
		kill(cur_pid, SIGSTOP);
		cout << "smash: process " << cur_pid << " was stopped" <<endl;
	}
	sigprocmask(SIG_SETMASK, &mask_set, &old_set);
}
