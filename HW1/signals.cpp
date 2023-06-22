// signals.cpp
// contains signal handler funtions
// contains the function/s that set the signal handlers

/*******************************************/
/* Name: handler_cntlc
   Synopsis: handle the Control-C */
#include "signals.h"
#define ERROR -1

extern int cur_pid;
extern int smash_pid;

// signal handler for SIGKILL
void ctrl_c_handler(int sig_num){
	sigset_t mask_set;
	sigset_t old_set;
	sigfillset(&mask_set);
	sigprocmask(SIG_SETMASK, &mask_set, &old_set);
	cout << "smash: caught ctrl-C" << endl;
	// sends SIGKILL only if the child process is running on foreground.
	if(cur_pid != smash_pid){
		int c_kill = kill(cur_pid, SIGKILL);
		if(c_kill == ERROR){
			perror("smash error: kill failed");
			return;
		}
		cout << "smash: process " << cur_pid << " was killed" << endl;
	}
	sigprocmask(SIG_SETMASK, &mask_set, &old_set);

}

// signal handler for SIGSTOP
void ctrl_z_handler(int sig_num){
	sigset_t mask_set;
	sigset_t old_set;
	sigfillset(&mask_set);
	sigprocmask(SIG_SETMASK, &mask_set, &old_set);
	cout << "smash: caught ctrl-Z" << endl;
	// sends SIGSTOP only if the child process is running on foreground.
	if(cur_pid != smash_pid){
		int z_kill = kill(cur_pid, SIGSTOP);
		if(z_kill == ERROR){
			perror("smash error: kill failed");
			return;
		}
		cout << "smash: process " << cur_pid << " was stopped" <<endl;
	}
	sigprocmask(SIG_SETMASK, &mask_set, &old_set);
}
