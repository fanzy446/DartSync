#include "../common/constants.h"

typedef struct p2p_request_pkg{
	char filename[FILE_NAME_LENGTH];
	unsigned long int timestamp;
	int partition;
} p2p_request_pkg_t;

typedef struct p2p_data_pkg{
	int code;
	char data[BLOCK_SIZE];
} p2p_data_pkg_t;

int download(char* filename, char** nodes, int numOfNodes);

int upload(p2p_request_pkg_t* pkg, /*char**/struct in_addr* ip, int port_num); 