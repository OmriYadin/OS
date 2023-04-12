//		commands.cpp
//********************************************
#include "commands.h"
//********************************************
// function name: ExeCmd
// Description: interperts and executes built-in commands
// Parameters: pointer to jobs, command string
// Returns: 0 - success,1 - failure
//**************************************************************************************

extern int cur_pid;
#define END 2;
char history[MAX_LINE_SIZE] = "0";

int Job::cur_serial = 0;

Job::Job(char command[], int id, double time, bool is_bg, bool stopped){
	cur_serial++;
	this->serial = cur_serial;
	strcpy(this->command, command);
	this->process_id = id;
	this->process_time = time;
	this->stopped = stopped;
	this->is_bg = is_bg;
}

void Job::update_serial(int new_serial){
	cur_serial = new_serial;
}




void list_update(list<Job> *jobs){
	list<Job>::iterator iter;
	int last_serial = 0;

	for(iter = jobs->begin(); iter != jobs->end(); iter++){
		int status;
		int pid_res = waitpid((pid_t)(iter->process_id), &status, WNOHANG);
		if (pid_res == -1){
			perror("smash error: waitpid failed");
			return;
		}
		if(WIFEXITED(status)){
			jobs->erase(iter);
			if (jobs->empty()){
				return;
			}
			iter--;
		}
		else{
			if(iter->serial > last_serial){
				last_serial = iter->serial;
			}
		}
	}
	iter->update_serial(last_serial);
}


int ExeCmd(list<Job> *jobs, char* lineSize, char* cmdString)
{
	char* cmd; 
	char* args[MAX_ARG];
	char pwd[MAX_LINE_SIZE];
	char delimiters[] = " \t\n";
	int i = 0, num_arg = 0;
	bool illegal_cmd = false; // illegal command
    cmd = strtok(lineSize, delimiters);
	if (cmd == NULL)
		return 0; 
   	args[0] = cmd;
	for (i=1; i<MAX_ARG; i++)
	{
		args[i] = strtok(NULL, delimiters); 
		if (args[i] != NULL) 
			num_arg++; 
 
	}

/*************************************************/
// Built in Commands PLEASE NOTE NOT ALL REQUIRED
// ARE IN THIS CHAIN OF IF COMMANDS. PLEASE ADD
// MORE IF STATEMENTS AS REQUIRED
/*************************************************/
	if (!strcmp(cmd, "cd") ) 
	{
		if (num_arg > 1){
			cout << "smash error: cd: too many arguments" << endl;;
		}
		else if (!strcmp(args[1], "..")){
			strcpy(history, getcwd(pwd, MAX_LINE_SIZE));
			chdir("..");
		}
		else if (strcmp(args[1], "-")){
			strcpy(history, getcwd(pwd, MAX_LINE_SIZE));
			chdir(args[1]);
		}
		else{
			if (!strcmp(history, "0")){
				cout << "smash error: cd: OLDPWD not set" << endl;
			}
			else{
				char history_tmp[MAX_LINE_SIZE];
				strcpy(history_tmp, getcwd(pwd, MAX_LINE_SIZE));
				chdir(history);
				strcpy(history, history_tmp);
			}
		}

	} 
	
	/*************************************************/
	else if (!strcmp(cmd, "pwd")) 
	{
		cout << getcwd(pwd, MAX_LINE_SIZE) << endl;
	}
	
	/*************************************************/
	else if (!strcmp(cmd, "mkdir"))
	{

	}
	/*************************************************/
	
	else if (!strcmp(cmd, "jobs")) 
	{

 		list_update(jobs);
 		list<Job>::iterator iter;
 		for (iter = jobs->begin(); iter != jobs->end(); iter++){
 			double job_time = difftime(time(NULL), iter->process_time);
 			cout << "[" << iter->serial << "] " << iter->command << " : "
 					<< iter->process_id << " " << job_time << " secs";
 			if (iter->stopped){
 				cout << " (stopped)";
 			}
 			cout << endl;
 		}
	}
	/*************************************************/
	else if (!strcmp(cmd, "showpid")) 
	{
		cout << "smash pid is " << getpid() << endl;
	}
	/*************************************************/
	else if (!strcmp(cmd, "kill"))
	{
		bool is_found = false;
		if (num_arg > 2){
			cout << "smash error: kill: invalid arguments" << endl;
		}
 		list<Job>::iterator iter_kill;
 		for (iter_kill = jobs->begin(); iter_kill != jobs->end(); iter_kill++){
 			if (iter_kill->serial == atoi(args[2])){
 				is_found = true;
 				if(!kill(iter_kill->process_id, abs(atoi(args[1])))){
 					cout << "signal number " << abs(atoi(args[1])) << "was sent to pid " << iter_kill->process_id << endl;
 				}
 				else{
 					cout << "smash error: kill: invalid arguments" << endl;
 				}
 			}
 		}
 		if (is_found == false){
 			cout << "smash error: kill: job-id " << args[2] << " does not exist" << endl;
 		}
	}
	/*************************************************/
	else if (!strcmp(cmd, "fg")) 
	{
		if (num_arg == 0){
			if (jobs->empty()){
				cout << "smash error: fg: jobs list is empty" << endl;
			}
			else {
				int process = jobs->end()->process_id;
				jobs->end()->is_bg = false;
				waitpid(process, NULL, 0);
			}
		}
		else if (num_arg == 1){
			int job = atoi(args[1]);
			if(job <= 0){
				cout << "smash error: fg: invalid arguments" << endl;
			}
			else{
				list<Job>::iterator iter;
				int process = 0;
				for(iter = jobs->begin(); iter != jobs->end(); iter++){
					if(iter->serial == job){
						process = iter->process_id;
						iter->is_bg = false;
						break;
					}
				}
				if (process == 0){
					cout << "smash error: fg: job-id " << job << " does not exist" << endl;
				}
				else {
					cout << process << endl;
					kill(process, SIGCONT);
					int status;
					cur_pid = process;
               		signal(SIGINT, ctrl_c_fg_handler);
               		signal(SIGTSTP, ctrl_z_fg_handler);
					int waitpid_res = waitpid(process, &status, WUNTRACED);
					if (waitpid_res == -1){
						perror("smash error: waitpid failed");
					}
					cout << "..." << endl;
					cur_pid = getpid();
               		signal(SIGINT, ctrl_c_smash_handler);
               		signal(SIGTSTP, ctrl_z_smash_handler);
               		if (WIFSTOPPED(status)){
               			iter->process_time = time(NULL);
               		}
				}
			}
		}
		else {
			cout << "smash error: fg: invalid arguments" << endl;
		}

	} 
	/*************************************************/
	else if (!strcmp(cmd, "bg")) 
	{
  		if (num_arg == 0){
  			list<Job>::iterator iter = jobs->end();
  			while ((iter != jobs->begin()) || (iter->stopped != true)){
  				iter--;
  			}
  			if (iter->stopped == true){
  				cout << iter->command << endl;
  				iter->stopped = false;
  				iter->is_bg = true;
  				kill(iter->process_id, SIGCONT);
  			}
  			else {
  				cout << "smash error: bg: there are no stopped jobs to resume" << endl;
  			}
  		}

  		else if (num_arg == 1){
  			int job = atoi(args[1]);
  			if (job <= 0){
  				cout << "smash error: bg: invalid arguments" << endl;
  			}
  			else {
				list<Job>::iterator iter = jobs->begin();
				int process = 0;
				while ((iter != jobs->end()) || (iter->serial != job)){
					iter++;
				}

				if (iter->serial != job){
					cout << "smash error: bg: job-id " << job << " does not exist" << endl;
				}
				else {
					if (iter->stopped != true){
						cout << "smash error: bg: job-id " << job << " is already running in the background" << endl;
					}
					else {
		  				cout << iter->command << endl;
		  				iter->stopped = false;
		  				iter->is_bg = true;
						kill(iter->process_id, SIGCONT);
					}
				}
  			}
  		}
  		else {
  			cout << "smash error: bg: invalid arguments" << endl;
  		}
	}
	/*************************************************/
	else if (!strcmp(cmd, "quit"))
	{
   		if (num_arg == 0){
   			return 2;
   		}
   		else{
   			if (!strcmp(args[1], "kill")){
   				list<Job>::iterator iter;
   				for (iter = jobs->begin(); iter != jobs->end(); iter++){
   					cout << "[" << iter->serial << "] " << iter->command << " - Sending SIGTERM...";
   					kill(iter->process_id, SIGTERM);
   					sleep(5);
   					int waitpid_res = waitpid((pid_t)(iter->process_id), NULL, WNOHANG);
   					if (waitpid_res == iter->process_id){
   						cout << " Done." << endl;
   					}
   					else if (waitpid_res == 0){
   						cout << " (5 sec passed) Sending SIGKILL...";
   						kill(iter->process_id, SIGKILL);
   						cout << " Done." << endl;
   					}
   					else{
   						perror("smash error: waitpid failed");
   					}
   				}
   				return END;
   			}
   			else {
   				return END;
   			}
   		}
	} 
	/*************************************************/
	else if (!strcmp(cmd, "diff")){
		if (num_arg != 2){
			cout << "smash error: diff: invalid arguments" << endl;
		}

		ifstream first(args[1]);
		ifstream second(args[2]);
		stringstream first_buf;
		stringstream second_buf;
		first_buf << first.rdbuf();
		second_buf << second.rdbuf();
		if(!first_buf.str().compare(second_buf.str())){
			cout << 1 << endl;
		}
		else {
			cout << 0 << endl;
		}

	}
	/************************************************/
	else // external command
	{
		list_update(jobs);
 		ExeExternal(args, cmdString, jobs, false);
	 	return 0;
	}
	if (illegal_cmd == true)
	{
		printf("smash error: > \"%s\"\n", cmdString);
		return 1;
	}
    return 0;
}
//**************************************************************************************
// function name: ExeExternal
// Description: executes external command
// Parameters: external command arguments, external command string
// Returns: void
//**************************************************************************************
void ExeExternal(char *args[MAX_ARG], char* cmdString, list<Job> *jobs, bool is_bg)
{
	int pID;
    switch(pID = fork())
	{
    		case -1: 
					// Add your code here (error)
    				perror("smash error: fork failed");
    				break;

        	case 0 :
                	// Child Process
               		setpgrp();
               		execvp(args[0], args);
               		exit(0);

			default:
					if (!is_bg){
						int status;
						cur_pid = pID;
						signal(SIGINT, ctrl_c_fg_handler);
						signal(SIGTSTP, ctrl_z_fg_handler);
						int waitpid_res = waitpid(pID, &status, WUNTRACED | WCONTINUED);
						if (waitpid_res == -1){
							perror("smash error: waitpid failed");
						}
						cur_pid = getpid();
						signal(SIGINT, ctrl_c_smash_handler);
						signal(SIGTSTP, ctrl_z_smash_handler);
						if (WIFSTOPPED(status)){
							jobs->push_back(
									Job(cmdString, pID, time(NULL), false, true));
						}
					}
					else{
               			jobs->push_back(
               				Job(cmdString, pID, time(NULL), true, false));
               		}

	}
}
//**************************************************************************************
// function name: ExeComp
// Description: executes complicated command
// Parameters: command string
// Returns: 0- if complicated -1- if not
//**************************************************************************************
int ExeComp(char* lineSize)
{
	char ExtCmd[MAX_LINE_SIZE+2];
	char *args[MAX_ARG];
    if ((strstr(lineSize, "|")) || (strstr(lineSize, "<")) || (strstr(lineSize, ">")) || (strstr(lineSize, "*")) || (strstr(lineSize, "?")) || (strstr(lineSize, ">>")) || (strstr(lineSize, "|&")))
    {
		// Add your code here (execute a complicated command)
					
		/* 
		your code
		*/
	} 
	return -1;
}
//**************************************************************************************
// function name: BgCmd
// Description: if command is in background, insert the command to jobs
// Parameters: command string, pointer to jobs
// Returns: 0- BG command -1- if not
//**************************************************************************************
int BgCmd(char* lineSize, list<Job> *jobs)
{

	list_update(jobs);
	char* cmd;
	char delimiters[] = " \t\n";
	char *args[MAX_ARG];
	if (lineSize[strlen(lineSize)-2] == '&')
	{
		char cmdString[MAX_LINE_SIZE];
		strcpy(cmdString, lineSize);
		cmdString[strlen(cmdString)-1] = '\0';
		lineSize[strlen(lineSize)-2] = '\0';
		int i = 0, num_arg = 0;
		bool illegal_cmd = false; // illegal command
	    cmd = strtok(lineSize, delimiters);
		if (cmd == NULL)
			return 0;
	   	args[0] = cmd;
		for (i=1; i<MAX_ARG; i++)
		{
			args[i] = strtok(NULL, delimiters);
		}
		ExeExternal(args, cmdString, jobs, true);

		return 0;
	}
	return -1;
}

