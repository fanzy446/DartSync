#ifndef P2P_H 
#define P2P_H

#include "../common/constants.h"

typedef struct p2p_request_pkg{
	char filename[FILE_NAME_LENGTH];
	unsigned long int timestamp;
	int partition;
} p2p_request_pkg_t;

typedef struct p2p_data_pkg{
	int size;
	char data[BLOCK_SIZE];
} p2p_data_pkg_t;

typedef struct p2p_request_arg{
	char ip[IP_LEN];
	char filename[FILE_NAME_LENGTH];
	unsigned long int timestamp;
	int partition;
	int* exist;
	int* running;
} p2p_request_arg_t;

typedef struct send_thread_arg
{
	int conn;
	int* running;
	p2p_request_arg_t *req_info;
}send_thread_arg_t;

int download(char* filename, int size, unsigned long int timestamp, char nodes[][IP_LEN], int numOfNodes);

void* start_listening(void* arg);

//////////////////////////////////

void* singleDownload(void* args);

int download_sendpkt(p2p_request_pkg_t* pkt, int conn);

int download_recvpkt(p2p_data_pkg_t* pkt, int conn);

int upload_sendpkt(p2p_data_pkg_t* pkt, int conn);

int upload_recvreqpkt(p2p_request_pkg_t* pkt, int conn);

void* upload_thd(void* arg);

int upload(int sockfd, p2p_request_pkg_t* pkg); 

#endif
