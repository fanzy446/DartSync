#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "p2p.h"

#define MAX_LISTEN_NUM 30

// typedef struct p2p_request_pkg{
// 	char filename[FILE_NAME_LENGTH];
// 	unsigned long int timestamp;
// 	int partition;
// } p2p_request_pkg_t;

// typedef struct p2p_data_pkg{
// 	int code;
// 	char data[BLOCK_SIZE];
// } p2p_data_pkg_t;

p2p_request_pkg_t test_pkg;



int init_listen_sock(int port){
	int listenfd, connfd, n;
	struct sockaddr_in cli_addr, serv_addr;
	socklen_t clilen;

	if ((listenfd = socket (AF_INET, SOCK_STREAM, 0)) <0)
	{
		perror("Error when creating listen socket");
		exit(1);
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(port);
	bind (listenfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
	listen (listenfd, MAX_LISTEN_NUM);
	printf("start listening!\n");
	while(1)
	{
		clilen = sizeof(cli_addr);
		connfd = accept(listenfd, (struct sockaddr *)&cli_addr, (socklen_t*)&clilen);
	    if (connfd < 0)
	    {
	        perror("accept failed");
	        return 1;
	    }
	    printf("Connection accepted\n");

	    char *buf = (char *)malloc(sizeof(char)*1024);
	    if((n=recv(connfd,buf,1,0))>0)
		{
			printf("port:%d\n", cli_addr.sin_port);
			printf("ip:%s\n", inet_ntoa(cli_addr.sin_addr));
			printf("recv request:%s\n", buf);

			upload(&test_pkg, /*inet_ntoa(*/&cli_addr.sin_addr/*)*/, cli_addr.sin_port);
		}
	}

}

int download(char* filename, char** nodes, int numOfNodes){
	return 1;
}

int upload(p2p_request_pkg_t* pkg, /*char**/struct in_addr* ip, int port_num){
	int partition = pkg->partition;

	FILE *fp;
	if((fp = fopen(pkg->filename,"r"))!=NULL){
		//read content to buffer
		char readBuf[BLOCK_SIZE];

		fseek(fp, (pkg->partition-1) * sizeof(char) * BLOCK_SIZE, SEEK_SET);
		fread(readBuf, sizeof(char), BLOCK_SIZE, fp);

		printf("%s\n", readBuf);
		fclose(fp);

		//create socket
		int sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0){
		    perror("sock");
		    return -1;
		}

		struct sockaddr_in addr;
  
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port_num);
		/*inet_aton(ip, &addr.sin_addr);*/
		memcpy(&addr.sin_addr, ip, sizeof(struct in_addr));
		//addr.sin_addr = inet_addr();
		  
		if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0){
		    perror("connect");
		    return -1;
		}

		printf("upload connected to destination\n");
		if(send(sockfd , readBuf , strlen(readBuf) , 0) < 0){
            puts("Send failed\n");
            return 1;
        }

	}else{
		printf("Can't open file!\n");
	}

	return 1;
}

int main(){
	strcpy(test_pkg.filename, "../test/test.txt");
	test_pkg.timestamp = 111111; // unused
	test_pkg.partition = 2;

	// int tmp = upload(&test_pkg);
	init_listen_sock(1234);
}