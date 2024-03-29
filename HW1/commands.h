#ifndef _COMMANDS_H
#define _COMMANDS_H
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sstream>
#include <fstream>
#include <string>
#include <math.h>
#include <errno.h>
#include <list>
#include <sys/types.h>
#include <sys/wait.h>
#include "signals.h"
#define MAX_LINE_SIZE 80
#define MAX_ARG 20
using namespace std;


// class for each process on the jobs list
class Job
{
	static int cur_serial;

	public:
	int serial;
	char command[MAX_LINE_SIZE];
	int process_id;
	double process_time;
	bool stopped;
	bool is_bg;

	Job(char command[], int id, double time, bool is_bg, bool stopped);
	void update_serial(int new_serial);
};



int BgCmd(char* lineSize, list<Job> *jobs);
int ExeCmd(list<Job> *jobs, char* lineSize, char* cmdString);
void ExeExternal(char *args[MAX_ARG], char* cmdString, list<Job> *jobs, bool is_bg);



#endif

