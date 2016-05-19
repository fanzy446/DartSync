typedef struct p2p_request_pkg{
	char* filename;
	unsigned long int timestamp;
	int partition;
} p2p_pkg_t;

typedef struct p2p_data_pkg{
	int code;
	char data[BLOCK_SIZE];
} p2p_pkg_t;

int download(char* filename, char** nodes, int numOfNodes);

int upload(p2p_pkg_t* pkg);