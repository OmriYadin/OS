#ifndef _SIGS_H
#define _SIGS_H
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>
#include <unistd.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
using namespace std;

void ctrl_c_smash_handler(int sig_num);
void ctrl_c_fg_handler(int sig_num);

void ctrl_z_smash_handler(int sig_num);
void ctrl_z_fg_handler(int sig_num);

#endif

