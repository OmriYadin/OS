#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include <string>
#include <string.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <fstream>
#include <errno.h>


#define BUF_MAX_LEN 516
#define PARAM_MAX_VAL 65535
#define ERROR 1
#define FATAL_ERR -1
#define NON_FATAL_ERR 0

using namespace std;

void udp_connection(int sockfd, unsigned short timeout, unsigned short max_resends);
int send_ack(int sockfd, int ack_num,struct sockaddr_in* cli_addr, socklen_t cli_addrlen);
int get_data(int sockfd, fstream* file, struct timeval tout, int* bl_num,
		fd_set* rd_set, char* file_name , unsigned short max_resends, int* err_cnt, 
		struct sockaddr_in* cli_addr, socklen_t cli_addrlen, unsigned short timeout);
int send_error(int sockfd, unsigned short code, char* msg,
		struct sockaddr_in* cli_addr, socklen_t cli_addrlen);
void sys_err(fstream* file, char* file_name);

// data structure for wrq packets 
struct pck_wrq {
	unsigned short opcode;
	char strings[BUF_MAX_LEN-2];
} __attribute__((packed));

// data structure for ack packets
struct pck_ack {
	unsigned short opcode;
	unsigned short bl_num;
} __attribute__((packed));

// data structure for data packets
struct pck_data {
	unsigned short opcode;
	unsigned short bl_num;
	char buffer[BUF_MAX_LEN-4];
} __attribute__((packed));

// data structure for error packets
struct pck_error {
	unsigned short opcode;
	unsigned short err_code;
	char err_msg[BUF_MAX_LEN-4];
} __attribute__((packed));


// the main function, responsible for creating usp connections.
int main(int argc, char** argv){
	if(argc != 4){
		cerr << "TTFTP_ERROR: illegal arguments" << endl;
		return ERROR;
	}
	int port_int = atoi(argv[1]);
	int timeout_int = atoi(argv[2]);
	int max_resends_int = atoi(argv[3]);
	if((port_int > PARAM_MAX_VAL) || (port_int <= 0) || (timeout_int > PARAM_MAX_VAL) ||
			(timeout_int <= 0) || (max_resends_int > PARAM_MAX_VAL) || (max_resends_int <= 0)){
		cerr << "TTFTP_ERROR: illegal arguments" << endl;
		return ERROR;
	}
	unsigned short port = (unsigned short)port_int;
	unsigned short timeout = (unsigned short)timeout_int;
	unsigned short max_resends = (unsigned short)max_resends_int;
	
	// initializing server socket
	struct sockaddr_in ser_addr;
	memset(&ser_addr, 0, sizeof(ser_addr));
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	ser_addr.sin_port = htons(port);
	
	// loop for each udp connection (each file)
	while(true){
		int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		if (sockfd == -1){
			sys_err(NULL, NULL);
		}
		if(bind(sockfd, (struct sockaddr*)&ser_addr, sizeof(ser_addr)) == -1){
			sys_err(NULL, NULL);
		}
		udp_connection(sockfd, timeout, max_resends);
		close(sockfd);
	}
	return 0;
}


// this function creates udp connection and then gets data in a loop until EOF
void udp_connection(int sockfd, unsigned short timeout, unsigned short max_resends){
	int num_count = 0;
	struct sockaddr_in cli_addr;
	memset(&cli_addr, 0, sizeof(cli_addr));
	socklen_t cli_addrlen = sizeof(cli_addr);
	struct pck_wrq wrq;
	if (recvfrom(sockfd, (void*)&wrq, sizeof(wrq), MSG_TRUNC,
			(struct sockaddr*)&cli_addr, &cli_addrlen) < 0){
		sys_err(NULL, NULL);
	}
	wrq.opcode = ntohs(wrq.opcode);
	
	// checks if the opcode is WRQ opcode
	if(wrq.opcode != 2){
		if(send_error(sockfd, 7, (char*)"Unknown user", &cli_addr, cli_addrlen) < 0)
			sys_err(NULL, NULL);
		return;
	}
	
	char file_name[BUF_MAX_LEN];
	strcpy(file_name, wrq.strings);
	if(!access(file_name, F_OK)){
		if(send_error(sockfd, 6, (char*)"File already exists", &cli_addr, cli_addrlen) < 0)
			sys_err(NULL, NULL);
		return;
	}
	else if (errno != ENOENT){
		sys_err(NULL, NULL);
	}
	fstream new_file;
	new_file.open(file_name, ios_base::out | ios_base::binary);
	
	// sends ack on WRQ
	if(send_ack(sockfd, num_count, &cli_addr, cli_addrlen) < 0){
		sys_err(&new_file, file_name);
	}
	struct timeval tout;
	fd_set rd_set;
	int err_cnt = 0;
	num_count++;
	
	// gets data until we get to EOF
	while(true){
		tout.tv_sec = timeout;
		tout.tv_usec = 0;
		FD_ZERO(&rd_set);
		FD_SET(sockfd, &rd_set);
		int data_rec =  get_data(sockfd, &new_file, (const struct timeval)tout, &num_count,
				&rd_set, file_name, max_resends, &err_cnt, &cli_addr, cli_addrlen, timeout);
		if(data_rec == FATAL_ERR){
			new_file.close();
			remove(file_name);
			break;
		}
		else if (data_rec == NON_FATAL_ERR){
			continue;
		}
		else if (data_rec < BUF_MAX_LEN){
			break;
		}
	}
	new_file.close();
	FD_CLR(sockfd, &rd_set);
}


// close the file and removes it in case of an syscall error
void sys_err(fstream* file, char* file_name){
	if(file != NULL){
		file->close();
		remove(file_name);
	}
	perror("TTFTP_ERROR");
	exit(ERROR);
}

// sends acks
int send_ack(int sockfd, int ack_num, struct sockaddr_in* cli_addr, socklen_t cli_addrlen){
	struct pck_ack ack;
	memset(&ack, 0, sizeof(ack));
	ack.opcode = htons(4);
	ack.bl_num = htons(ack_num);
	if(sendto(sockfd, (const void*)&ack, sizeof(ack), MSG_CONFIRM,
			(const struct sockaddr*)cli_addr, cli_addrlen) < 0){
		return -1;
	}
	return 0;
}

// sends errors
int send_error(int sockfd, unsigned short code, char* msg,
		struct sockaddr_in* cli_addr, socklen_t cli_addrlen){
	struct pck_error err;
	memset(&err, 0, sizeof(err));
	err.opcode = htons(5);
	err.err_code = htons(code);
	strcpy(err.err_msg, msg);
	if(sendto(sockfd, (const void*)&err, sizeof(err), MSG_CONFIRM,
			(const struct sockaddr*)cli_addr, cli_addrlen) < 0){
		return -1;
	}
	return 0;
}

// gets data
int get_data(int sockfd, fstream* file, struct timeval tout, int* bl_num,
		fd_set* rd_set, char* file_name, unsigned short max_resends, int* err_cnt,
		struct sockaddr_in* cli_addr, socklen_t cli_addrlen, unsigned short timeout){
	int tmp = select(sockfd+1, NULL, rd_set, NULL, &tout);
	// runs until we get packet or reach the max number of resends
	while(tmp <= 0){
		if(tmp < 0){
			sys_err(file, file_name);
		}
		*err_cnt += 1;
		if(*err_cnt > max_resends){
			if(send_error(sockfd, 0, (char*)"Abandoning file transmission",
					cli_addr, cli_addrlen) < 0)
				sys_err(file, file_name);
			return FATAL_ERR;
		}
		if(send_ack(sockfd, (*bl_num)-1, cli_addr, cli_addrlen) < 0){
			sys_err(file, file_name);
		}
		tout.tv_sec = timeout;
		tout.tv_usec = 0;
		FD_ZERO(rd_set);
		FD_SET(sockfd, rd_set);
		tmp = select(sockfd+1, NULL, rd_set, NULL, &tout);
	}
	struct pck_data data;
	struct sockaddr_in tmp_addr;	
	memset(&tmp_addr, 0, sizeof(tmp_addr));
	socklen_t tmp_addrlen = sizeof(tmp_addr);
	
	// reads data from socket
	ssize_t bytes_rec = recvfrom(sockfd, (void*)&data, sizeof(data), MSG_TRUNC,
			(struct sockaddr*)&tmp_addr, &tmp_addrlen);
	if (bytes_rec <= 0){
		sys_err(file, file_name);
	}

	// checks if the data packet is from the correct client 
	if((tmp_addr.sin_port != cli_addr->sin_port) ||
			(tmp_addr.sin_addr.s_addr != cli_addr->sin_addr.s_addr)){
		if(send_error(sockfd, 4, (char*)"Unexpected packet", &tmp_addr, tmp_addrlen) < 0)
			sys_err(file, file_name);

		return NON_FATAL_ERR;
	}
	data.opcode = ntohs(data.opcode);
	data.bl_num = ntohs(data.bl_num);
	
	// checks if the opcode is correct
	if(data.opcode != 3){
		if(send_error(sockfd, 4, (char*)"Unexpected packet", cli_addr, cli_addrlen) < 0)
			sys_err(file, file_name);
	}
	
	// checks if we got the same block because of ack that fell
	else if(data.bl_num == (*bl_num)-1){
		if(send_ack(sockfd, (*bl_num)-1, cli_addr, cli_addrlen) < 0)
			sys_err(file, file_name);
		*err_cnt += 1;
		if(*err_cnt > max_resends){
			if(send_error(sockfd, 0, (char*)"Abandoning file transmission", cli_addr, cli_addrlen) < 0)
				sys_err(file, file_name);
			return FATAL_ERR;
		}
		return NON_FATAL_ERR;
	}
	
	// sends fatal error because the block is not the same as before and not a new one
	else if (data.bl_num != *bl_num){
		if(send_error(sockfd, 0, (char*)"Bad block number", cli_addr, cli_addrlen) < 0)
			sys_err(file, file_name);
		return FATAL_ERR;
	}
	
	// in case everything is ok, we write to file
	else {
		file->write(data.buffer, bytes_rec - 4);
		if(send_ack(sockfd, (*bl_num), cli_addr, cli_addrlen) < 0){
			sys_err(file, file_name);
		}
		*bl_num += 1;
		*err_cnt = 0;
	}
	return bytes_rec;
}
