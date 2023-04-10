//		commands.cpp
//********************************************
#include "commands.h"
//********************************************
// function name: ExeCmd
// Description: interperts and executes built-in commands
// Parameters: pointer to jobs, command string
// Returns: 0 - success,1 - failure
//**************************************************************************************

char history[MAX_LINE_SIZE] = "0";
static int j = 1;

class Job
{
	static int cur_serial;

	public:
	int serial;
	char command[MAX_LINE_SIZE];
	int proccess_id;
	double proccess_time;
	bool stopped;
	bool finished;

	Job(char command[], int id, double time){
		cur_serial++;
		this->serial = cur_serial;
		strcpy(this->command, command);
		this->proccess_id = id;
		this->proccess_time = time;
		this->stopped = false;
		this->finished = false;
	}

	void update_serial(int new_serial){
		cur_serial = new_serial;
	}
};

int Job::cur_serial = 0;
std::list <Job> jobs;

void list_update(){
	list<Job>::iterator iter;
	int last_serial = 0;
	for(iter = jobs.end(); iter != jobs.begin(); iter--){
		if(waitpid((pid_t)(iter->proccess_id), NULL, WNOHANG)){
			jobs.erase(iter);
		}
		else{
			if(iter->serial > last_serial){
				last_serial = iter->serial;
			}
		}
	}
	iter->update_serial(last_serial);
}


int ExeCmd(void* jobs, char* lineSize, char* cmdString)
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
 		list_update();
 		list<Job>::iterator iter;
 		for (iter = jobs.begin(); iter != jobs.end(); iter++){
 			double job_time = difftime(time(NULL), iter->proccess_time);
 			cout << "[" << iter->proccess_id << "] " << iter->command << " : "
 					<< iter->proccess_id << job_time << "secs";
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
 		for (iter_kill = jobs.begin(); iter_kill != jobs.end(); iter_kill++){
 			if (iter_kill->serial == atoi(args[2])){
 				is_found = true;
 				if(!kill(iter_kill->proccess_id, abs(atoi(args[1])))){
 					cout << "signal number " << abs(atoi(args[1])) << "was sent to pid " << iter_kill->proccess_id << endl;
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
		
	} 
	/*************************************************/
	else if (!strcmp(cmd, "bg")) 
	{
  		
	}
	/*************************************************/
	else if (!strcmp(cmd, "quit"))
	{
   		
	} 
	/*************************************************/
	else // external command
	{
 		ExeExternal(args, cmdString);
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
void ExeExternal(char *args[MAX_ARG], char* cmdString)
{
	int pID;
	list_update();
    switch(pID = fork())
	{
    		case -1: 
					// Add your code here (error)
					
					/* 
					your code
					*/
    				break;

        	case 0 :
                	// Child Process
               		setpgrp();

               		break;

			default:

					
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
int BgCmd(char* lineSize, void* jobs)
{

	char* Command;
	char delimiters[] = " \t\n";
	char *args[MAX_ARG];
	if (lineSize[strlen(lineSize)-2] == '&')
	{
		lineSize[strlen(lineSize)-2] = '\0';
		jobs.push_back(Job(cmdString, pID, time(NULL)));
		
	}
	return -1;
}

