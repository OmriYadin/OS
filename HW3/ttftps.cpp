#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include <string>
#include <netinet/in.h>
#include <select.h>

#define BUF_MAX_LEN 516
#define PARAM_MAX_VAL 65535
#define ERROR 1

using namespace std;

void udp_connection(int sockfd, unsigned short timeout, unsigned short max_resends);
void wrq_check(int sockfd, struct socketaddr_in* cli_addr, socklen_t cli_addrlen);
void file_exists(int sockfd, struct socketaddr_in* cli_addr, socklen_t cli_addrlen);
void send_ack(int sockfd, int ack_num);
int get_data(int sockfd, fstream* file, const struct timeval tout, int bl_num);

struct pck_wrq {
	unsigned short opcode;
	string file_name;
	string mode;
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
	string err_msg;
} __attribute__((packed));


int main(int argc, char* argv){
	//char buffer[BUF_MAX_LEN] = {0};
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
	struct sockaddr_in ser_addr = {0};
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_addr.s_addr = INADDR_ANY;
	ser_addr.sin_port = htons(port);
	//struct sockaddr_in cli_addr;
	
	while(true){
		int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		if (sockfd == -1){
			perror("TTFTP_ERROR:");
			return ERROR;
		}
		if(bind(sockfd, (struct sockaddr*)&ser_addr, sizeof(ser_addr)) == -1){
			perror("TTFTP_ERROR:");
			return ERROR;
		}
		udp_connection(sock_fd, timeout, max_resends);
		close(sock_fd);
	}
	return 0;
}


void udp_connection(int sockfd, unsigned short timeout, unsigned short max_resends){
	int num_count = 0;
	struct socketaddr_in cli_addr = {0};
	socklen_t cli_addrlen;
	struct pck_wrq wrq;
	ssize_t packet_size = recvfrom(sockfd, (void*)&wrq, BUF_MAX_LEN, 0, 
			(struct sockaddr*)&cli_addr, cli_addrlen);
	if (packet_size <= 0){
		perror("TTFTP_ERROR:");
		exit(ERROR);
	}
	if(wrq.opcode != 2){
		wrq_check(sockfd, &cli_addr, cli_addrlen);
		return;
	}
	if(access(wrq.file_name) < 0){
		file_exists(sockfd, &cli_addr, cli_addrlen);
	}
	if(connect(sockfd, (struct sockaddr*)&cli_addr, &cli_addrlen) < 0){
		perror("TTFTP_ERROR:");
		exit(ERROR);
	}
	fstream new_file;
	new_file.open(wrq.file_name, ios_base::out);
	send_ack(sockfd, 0);
	struct timeval tout;
	tout.tv_sec = timeout;
	tout.tv_usec = 0;
	while(true){
		
	}
	
	
	
	
	
	
}


void wrq_check(int sockfd, struct socketaddr_in* cli_addr, socklen_t cli_addrlen){
	struct pck_error err;
	err.opcode = 5;
	err.err_code = 7;
	err.err_msg = "Unknown user";
	if(sendto(sockfd, (const void*)&err, sizeof(err), 0, 
			(const struct sockaddr*)cli_addr, cli_addrlen) < 0){
		perror("TTFTP_ERROR:");
		exit(ERROR);
	}
	return;
}


void file_exists(int sockfd, struct socketaddr_in* cli_addr, socklen_t cli_addrlen){
	struct pck_error err;
	err.opcode = 5;
	err.err_code = 6;
	err.err_msg = "File already exists";
	if(sendto(sockfd, (const void*)&err, sizeof(err), 0, 
			(const struct sockaddr*)cli_addr, cli_addrlen) < 0){
		perror("TTFTP_ERROR:");
		exit(ERROR);
	}
	return;
}


void send_ack(int sockfd, int ack_num){
	struct pck_ack ack;
	ack.opcode = 4;
	ack.bl_num = ack_num;
	if(send(sockfd, (const void*)&ack, sizeof(ack), 0) < 0){
		perror("TTFTP_ERROR:");
		exit(ERROR);
	}
	return;
}


int get_data(int sockfd, fstream* file, const struct timeval tout, int bl_num){
	fd_set rd_set;
}
