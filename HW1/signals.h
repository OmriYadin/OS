#ifndef _SIGS_H
#define _SIGS_H
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>
#include <unistd.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
using namespace std;

void ctrl_c_handler(int sig_num);

void ctrl_z_handler(int sig_num);

#endif

