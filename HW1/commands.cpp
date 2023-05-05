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
extern int smash_pid;
#define END 2
#define ERROR -1
#define FAILURE 1
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
	if (jobs->empty()){
		jobs->begin()->update_serial(0);
		return;
	}
	for(iter = jobs->begin(); iter != jobs->end(); iter++){
		int status;
		int pid_res = waitpid((pid_t)(iter->process_id), &status, WNOHANG);
		if (pid_res == ERROR){
			if(errno == ECHILD){
				jobs->erase(iter);
				if (jobs->empty()){
					iter->update_serial(0);
					return;
				}
				iter--;
			}
			else{
				perror("smash error: waitpid failed");
			}
			continue;
		}
		if(WIFEXITED(status)){
			jobs->erase(iter);
			if (jobs->empty()){
				iter->update_serial(0);
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
			cerr << "smash error: cd: too many arguments" << endl;;
		}
		else if (!strcmp(args[1], "..")){
			if (!getcwd(pwd, MAX_LINE_SIZE)){
				perror("smash error: getcwd failed");
				return FAILURE;
			}
			int res = chdir("..");
			if (res == ERROR){
				perror("smash error: chdir failed");
				return FAILURE;
			}
			strcpy(history, pwd);
		}
		else if (strcmp(args[1], "-")){
			if (!getcwd(pwd, MAX_LINE_SIZE)){
				perror("smash error: getcwd failed");
				return FAILURE;
			}
			int res = chdir(args[1]);
			if (res == ERROR){
				perror("smash error: chdir failed");
				return FAILURE;
			}
			strcpy(history, pwd);

		}
		else{
			if (!strcmp(history, "0")){
				cerr << "smash error: cd: OLDPWD not set" << endl;
			}
			else{
				if (!getcwd(pwd, MAX_LINE_SIZE)){
					perror("smash error: getcwd failed");
					return FAILURE;
				}
				int res = chdir(history);
				if (res == ERROR){
					perror("smash error: chdir failed");
					return FAILURE;
				}
				strcpy(history, pwd);
			}
		}
	} 
	
	/*************************************************/
	else if (!strcmp(cmd, "pwd")) 
	{
		if (getcwd(pwd, MAX_LINE_SIZE) == NULL){
			perror("smash error: getcwd failed");
			return FAILURE;
		}
		cout << pwd << endl;
	}
	
	/*************************************************
	else if (!strcmp(cmd, "mkdir"))
	{

	}
	*************************************************/
	
	else if (!strcmp(cmd, "jobs")) 
	{
 		list_update(jobs);
 		list<Job>::iterator iter;
 		for (iter = jobs->begin(); iter != jobs->end(); iter++){
 			double jobs_time = time(NULL);
 			if (jobs_time == ERROR){
 				perror("smash error: time failed");
 				return FAILURE;
 			}
 			double job_time = difftime(jobs_time, iter->process_time);
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
		if (num_arg != 2){
			cerr << "smash error: kill: invalid arguments" << endl;
		}
 		list<Job>::iterator iter_kill;
 		for (iter_kill = jobs->begin(); iter_kill != jobs->end(); iter_kill++){
 			if (iter_kill->serial == atoi(args[2])){
 				is_found = true;
 				int kill_check = kill(iter_kill->process_id, abs(atoi(args[1])));
 				if (kill_check == ERROR){
 					if (errno == EINVAL || errno == ESRCH){
 						cerr << "smash error: kill: invalid arguments" << endl;
 						return FAILURE;
 					}
 					perror("smash error: kill failed");
 					return FAILURE;
 				}
 				if(!kill_check){
 					cout << "signal number " << abs(atoi(args[1])) <<
 							" was sent to pid " << iter_kill->process_id << endl;
 				}
 			}
 		}
 		if (is_found == false){
 			cerr << "smash error: kill: job-id " << args[2] << " does not exist" << endl;
 		}
	}
	/*************************************************/
	else if (!strcmp(cmd, "fg")) 
	{
		if (num_arg == 0){
			if (jobs->empty()){
				cerr << "smash error: fg: jobs list is empty" << endl;
			}
			else {
				int process = jobs->rbegin()->process_id;
				jobs->rbegin()->is_bg = false;
				if(jobs->rbegin()->stopped){
					if(kill(process, SIGCONT) == ERROR){
						perror("smash error: kill failed");
						return FAILURE;
					}
				}
				int status;
				cur_pid = process;
				cout << jobs->rbegin()->command << " : " << process << endl;
				int waitpid_res = waitpid(process, &status, WUNTRACED);
				if (waitpid_res == ERROR){
					perror("smash error: waitpid failed");
					return FAILURE;
				}
				cur_pid = getpid();
				if (WIFSTOPPED(status)){
					jobs->rbegin()->stopped = true;
					jobs->rbegin()->process_time = time(NULL);
				}
			}
		}
		else if (num_arg == 1){
			int job = atoi(args[1]);
			if(job <= 0){
				cerr << "smash error: fg: invalid arguments" << endl;
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
					cerr << "smash error: fg: job-id " << job << " does not exist" << endl;
				}
				else {
					cur_pid = process;
					if(iter->stopped){
						if(kill(process, SIGCONT) == ERROR){
							perror("smash error: kill failed");
							return FAILURE;
						}
					}
					int status;
					cout << iter->command << " : "
					 					<< iter->process_id << endl;
					int waitpid_res = waitpid(process, &status, WUNTRACED);
					if (waitpid_res == ERROR){
						perror("smash error: waitpid failed");
						return FAILURE;
					}
					cur_pid = getpid();
               		if (WIFSTOPPED(status)){
               			iter->stopped = true;
               			iter->process_time = time(NULL);
               		}
				}
			}
		}
		else {
			cerr << "smash error: fg: invalid arguments" << endl;
		}

	} 
	/*************************************************/
	else if (!strcmp(cmd, "bg")) 
	{
  		if (num_arg == 0){
  			list<Job>::iterator iter = jobs->end();
  			if (!(jobs->empty())){
				while ((iter != jobs->begin()) || (iter->stopped != true)){
					iter--;
				}
				if (iter->stopped == true){
					cout << iter->command << " : " << iter->process_id << endl;
					iter->stopped = false;
					iter->is_bg = true;
					int bg_kill = kill(iter->process_id, SIGCONT);
					if (bg_kill == ERROR){
						perror("smash error: kill failed");
						return FAILURE;
					}
				}
	  			else {
	  				cerr << "smash error: bg: there are no stopped jobs to resume" << endl;
	  			}
  			}
  			else {
  				cerr << "smash error: bg: there are no stopped jobs to resume" << endl;
  			}
  		}

  		else if (num_arg == 1){
  			int job = atoi(args[1]);
  			if (job <= 0){
  				cerr << "smash error: bg: invalid arguments" << endl;
  			}
  			else {
				list<Job>::iterator iter = jobs->begin();
				while ((iter != jobs->end()) && (iter->serial != job)){
					iter++;
				}

				if (iter->serial != job){
					cerr << "smash error: bg: job-id " << job << " does not exist" << endl;
				}
				else {
					if (iter->stopped != true){
						cerr << "smash error: bg: job-id " << job <<
								" is already running in the background" << endl;
					}
					else {
		  				cout << iter->command << " : " << iter->process_id << endl;
		  				iter->stopped = false;
		  				iter->is_bg = true;
		  				int bg_kill = kill(iter->process_id, SIGCONT);
		  		  		if (bg_kill == ERROR){
		  		  			perror("smash error: kill failed");
		  		  			return FAILURE;
		  		  		}
					}
				}
  			}
  		}
  		else {
  			cerr << "smash error: bg: invalid arguments" << endl;
  		}
	}
	/*************************************************/
	else if (!strcmp(cmd, "quit"))
	{
   		if (num_arg == 0){
   			return END;
   		}
   		else{
   			if (!strcmp(args[1], "kill")){
   				list<Job>::iterator iter;
   				int status;
   				for (iter = jobs->begin(); iter != jobs->end(); iter++){
   					cout << "[" << iter->serial << "] " << iter->command << " - Sending SIGTERM..." << flush;
   					int quit_kill = kill(iter->process_id, SIGTERM);
   					if(quit_kill == ERROR){
   						perror("smash error: kill failed");
   						return FAILURE;
   					}
   					
   					int waitpid_res;
   					for(int i = 0; i < 5; i++){
   						sleep(1);
   						waitpid_res = waitpid((pid_t)(iter->process_id), &status, WNOHANG);
						if (WIFSIGNALED(status) && waitpid_res > 0){
							cout << " Done." << endl;
							break;
						}
						else if (waitpid_res == ERROR){
	   						perror("smash error: waitpid failed");
						}
   					}
   					if (waitpid_res == 0){
   						cout << " (5 sec passed) Sending SIGKILL...";
   						int second_kill = kill(iter->process_id, SIGKILL);
   						if (second_kill == ERROR){
   							perror("smash error: kill failed");
   						}
   						cout << " Done." << endl;
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
			cerr << "smash error: diff: invalid arguments" << endl;
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
		return FAILURE;
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
    				exit(1);

        	case 0 :
                	// Child Process
               		setpgrp();
               		execvp(args[0], args);
               		perror("smash error: exec failed");
               		exit(1);

			default:
					if (!is_bg){
						int status;
						cur_pid = pID;
						int waitpid_res = waitpid(pID, &status, WUNTRACED | WCONTINUED);
						if (waitpid_res == ERROR){
							perror("smash error: waitpid failed");
						}
						cur_pid = getpid();
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
// Returns: 0- if complicated ERROR- if not
//**************************************************************************************
/*int ExeComp(char* lineSize)
{
	char ExtCmd[MAX_LINE_SIZE+2];
	char *args[MAX_ARG];
    if ((strstr(lineSize, "|")) || (strstr(lineSize, "<")) || (strstr(lineSize, ">")) || (strstr(lineSize, "*")) || (strstr(lineSize, "?")) || (strstr(lineSize, ">>")) || (strstr(lineSize, "|&")))
    {
		// Add your code here (execute a complicated command)
					
	} 
	return ERROR;
}*/
//**************************************************************************************
// function name: BgCmd
// Description: if command is in background, insert the command to jobs
// Parameters: command string, pointer to jobs
// Returns: 0- BG command ERROR- if not
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
		int i = 0;
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
	return ERROR;
}

