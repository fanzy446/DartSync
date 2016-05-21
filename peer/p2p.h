#ifndef P2P_H 
#define P2P_H

#include "../common/constants.h"
#include "../common/filetable.h"

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
	char* ip;
	char* filename;
	unsigned long int timestamp;
	int partition;
} p2p_request_arg_t;

int download(char* filename, int size, unsigned long int timestamp, char** nodes, int numOfNodes);

int singleDownload(void* args);

int download_sendpkt(p2p_request_pkg_t* pkt, int conn);

int download_recvpkt(p2p_data_pkg_t* pkt, int conn);

int upload(int sockfd, p2p_request_pkg_t* pkg); 
#endif
