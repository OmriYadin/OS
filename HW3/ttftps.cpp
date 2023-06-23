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
void wrq_check(int sockfd, struct sockaddr_in* cli_addr, socklen_t cli_addrlen);
void file_exists(int sockfd, struct sockaddr_in* cli_addr, socklen_t cli_addrlen);
void send_ack(int sockfd, int ack_num, fstream* file, char* file_name, 
		struct sockaddr_in* cli_addr, socklen_t cli_addrlen);
int get_data(int sockfd, fstream* file, struct timeval tout, int* bl_num,
		fd_set* rd_set, char* file_name , unsigned short max_resends, int* err_cnt, 
		struct sockaddr_in* cli_addr, socklen_t cli_addrlen, unsigned short timeout);
void send_error(int sockfd, unsigned short code, char* msg,
		fstream* file, char* file_name, struct sockaddr_in* cli_addr, socklen_t cli_addrlen);

struct pck_wrq {
	unsigned short opcode;
	char strings[BUF_MAX_LEN-2];
} __attribute__((packed));


struct pck_ack {
	unsigned short opcode;
	unsigned short bl_num;
} __attribute__((packed));


struct pck_data {
	unsigned short opcode;
	unsigned short bl_num;
	char buffer[BUF_MAX_LEN-4];
} __attribute__((packed));


struct pck_error {
	unsigned short opcode;
	unsigned short err_code;
	char err_msg[BUF_MAX_LEN-4];
} __attribute__((packed));


int main(int argc, char** argv){
	if(argc != 4){
		cerr << "TTFTP_ERROR illegal arguments" << endl;
		return ERROR;
	}
	int port_int = atoi(argv[1]);
	int timeout_int = atoi(argv[2]);
	int max_resends_int = atoi(argv[3]);
	if((port_int > PARAM_MAX_VAL) || (port_int <= 0) || (timeout_int > PARAM_MAX_VAL) ||
			(timeout_int <= 0) || (max_resends_int > PARAM_MAX_VAL) || (max_resends_int <= 0)){
		cerr << "TTFTP_ERROR illegal arguments" << endl;
		return ERROR;
	}
	unsigned short port = (unsigned short)port_int;
	unsigned short timeout = (unsigned short)timeout_int;
	unsigned short max_resends = (unsigned short)max_resends_int;
	struct sockaddr_in ser_addr;
	memset(&ser_addr, 0, sizeof(ser_addr));
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	ser_addr.sin_port = htons(port);
	//struct sockaddr_in cli_addr;
	
	while(true){
		int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		if (sockfd == -1){
			perror("TTFTP_ERROR");
			return ERROR;
		}
		if(bind(sockfd, (struct sockaddr*)&ser_addr, sizeof(ser_addr)) == -1){
			perror("TTFTP_ERROR");
			return ERROR;
		}
		udp_connection(sockfd, timeout, max_resends);
		close(sockfd);
	}
	return 0;
}


void udp_connection(int sockfd, unsigned short timeout, unsigned short max_resends){
	int num_count = 0;
	struct sockaddr_in cli_addr;
	memset(&cli_addr, 0, sizeof(cli_addr));
	socklen_t cli_addrlen = sizeof(cli_addr);
	struct pck_wrq wrq;
	memset(&wrq, 0, sizeof(wrq));
	ssize_t packet_size = recvfrom(sockfd, (void*)&wrq, sizeof(wrq), MSG_TRUNC, 
			(struct sockaddr*)&cli_addr, &cli_addrlen);
	wrq.opcode = ntohs(wrq.opcode);
	if (packet_size <= 0){
		perror("TTFTP_ERROR");
		exit(ERROR);
	}
	if(wrq.opcode != 2){
		wrq_check(sockfd, &cli_addr, cli_addrlen);
		return;
	}
	char* file_name = strtok(wrq.strings, "\0");
	int acc = access(file_name, F_OK);
	if(!acc){
		file_exists(sockfd, &cli_addr, cli_addrlen);
		return;
	}
	else if (errno != ENOENT){
		perror("TTFTP_ERROR");
		exit(ERROR);
	}
	fstream new_file;
	new_file.open(file_name, ios_base::out);
	send_ack(sockfd, num_count, &new_file, file_name, &cli_addr, cli_addrlen);
	struct timeval tout;
	fd_set rd_set;
	int err_cnt = 0;
	num_count++;
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
	FD_CLR(sockfd, &rd_set);
}


void wrq_check(int sockfd, struct sockaddr_in* cli_addr, socklen_t cli_addrlen){
	struct pck_error err;
	memset(&err, 0, sizeof(err));
	err.opcode = htons(5);
	err.err_code = htons(7);
	strcpy(err.err_msg, "Unknown user");
	if(sendto(sockfd, (const void*)&err, sizeof(err), MSG_CONFIRM, 
			(const struct sockaddr*)cli_addr, cli_addrlen) < 0){
		perror("TTFTP_ERROR");
		exit(ERROR);
	}
	return;
}


void file_exists(int sockfd, struct sockaddr_in* cli_addr, socklen_t cli_addrlen){
	struct pck_error err;
	memset(&err, 0, sizeof(err));
	err.opcode = htons(5);
	err.err_code = htons(6);
	strcpy(err.err_msg, "File already exists");
	if(sendto(sockfd, (const void*)&err, sizeof(err), MSG_CONFIRM, 
			(const struct sockaddr*)cli_addr, cli_addrlen) < 0){
		perror("TTFTP_ERROR");
		exit(ERROR);
	}
	return;
}


void send_ack(int sockfd, int ack_num, fstream* file, char* file_name,
		struct sockaddr_in* cli_addr, socklen_t cli_addrlen){
	struct pck_ack ack;
	memset(&ack, 0, sizeof(ack));
	ack.opcode = htons(4);
	ack.bl_num = htons(ack_num);
	if(sendto(sockfd, (const void*)&ack, sizeof(ack), MSG_CONFIRM,
			(const struct sockaddr*)cli_addr, cli_addrlen) < 0){
		file->close();
		remove(file_name);
		perror("TTFTP_ERROR");
		exit(ERROR);
	}
	return;
}



void send_error(int sockfd, unsigned short code, char* msg,
		fstream* file, char* file_name,
		struct sockaddr_in* cli_addr, socklen_t cli_addrlen){
	struct pck_error err;
	memset(&err, 0, sizeof(err));
	err.opcode = htons(5);
	err.err_code = htons(code);
	strcpy(err.err_msg, msg);
	if(sendto(sockfd, (const void*)&err, sizeof(err), MSG_CONFIRM,
			(const struct sockaddr*)cli_addr, cli_addrlen) < 0){
		file->close();
		remove(file_name);
		perror("TTFTP_ERROR");
		exit(ERROR);
	}
	return;
}

int get_data(int sockfd, fstream* file, struct timeval tout, int* bl_num,
		fd_set* rd_set, char* file_name, unsigned short max_resends, int* err_cnt,
		struct sockaddr_in* cli_addr, socklen_t cli_addrlen, unsigned short timeout){
	int tmp = select(sockfd+1, NULL, rd_set, NULL, &tout);
	while(tmp <= 0){
		if(tmp < 0){
			file->close();
			remove(file_name);
			perror("TTFTP_ERROR");
			exit(ERROR);
		}
		*err_cnt += 1;
		if(*err_cnt > max_resends){
			send_error(sockfd, 0, (char*)"Abandoning file transmission", file, file_name,
					cli_addr, cli_addrlen);
			return FATAL_ERR;
		}
		send_ack(sockfd, (*bl_num)-1, file, file_name, cli_addr, cli_addrlen);
		tout.tv_sec = timeout;
		tout.tv_usec = 0;
		FD_ZERO(rd_set);
		FD_SET(sockfd, rd_set);
		tmp = select(sockfd+1, NULL, rd_set, NULL, &tout);
	}
	struct pck_data data;
	memset(&data, 0, sizeof(data));
	struct sockaddr_in tmp_addr;	
	memset(&tmp_addr, 0, sizeof(tmp_addr));
	socklen_t tmp_addrlen = sizeof(tmp_addr);
	ssize_t bytes_rec = recvfrom(sockfd, (void*)&data, sizeof(data), MSG_TRUNC, 
			(struct sockaddr*)&tmp_addr, &tmp_addrlen);
	if (bytes_rec <= 0){
		perror("TTFTP_ERROR");
		exit(ERROR);
	}

	if((tmp_addr.sin_port != cli_addr->sin_port) ||
			(tmp_addr.sin_addr.s_addr != cli_addr->sin_addr.s_addr)){
		send_error(sockfd, 4, (char*)"Unexpected packet", file, file_name, &tmp_addr, tmp_addrlen);
		return NON_FATAL_ERR;
	}
	data.opcode = ntohs(data.opcode);
	data.bl_num = ntohs(data.bl_num);
	if(data.opcode != 3){
		send_error(sockfd, 4, (char*)"Unexpected packet", file, file_name, cli_addr, cli_addrlen);
	}
	else if(data.bl_num == (*bl_num)-1){
		send_error(sockfd, 0, (char*)"Bad block number", file, file_name, cli_addr, cli_addrlen);
		*err_cnt += 1;
		if(*err_cnt > max_resends){
			send_error(sockfd, 0, (char*)"Abandoning file transmission", file, file_name, cli_addr, cli_addrlen);
			return FATAL_ERR;
		}
		return NON_FATAL_ERR;
	}
	else if (data.bl_num != *bl_num){
		send_error(sockfd, 0, (char*)"Bad block number", file, file_name, cli_addr, cli_addrlen);
		return FATAL_ERR;
	}
	else {
		*file << data.buffer;
		send_ack(sockfd, (*bl_num), file, file_name, cli_addr, cli_addrlen);
		*bl_num += 1;
		*err_cnt = 0;
	}
	return bytes_rec;
}
