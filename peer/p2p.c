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
#define LISTENING_PORT 1234

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
	bind(listenfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
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
	    if((n=recv(connfd, buf, 1024, 0))>0){	
			printf("from ip:%s | port:%d\n", inet_ntoa(cli_addr.sin_addr), cli_addr.sin_port);
			printf("recv request:%s\n", buf);

			upload(connfd, &test_pkg);
		}
	}

}

int download(char* filename, char** nodes, int numOfNodes){
	return 1;
}

int upload(int sockfd, p2p_request_pkg_t* pkg){
	int partition = pkg->partition;

	FILE *fp;
	if((fp = fopen(pkg->filename,"r"))!=NULL){
		//read content to buffer
		char readBuf[BLOCK_SIZE];

		fseek(fp, (pkg->partition-1) * sizeof(char) * BLOCK_SIZE, SEEK_SET);
		fread(readBuf, sizeof(char), BLOCK_SIZE, fp);

		//printf("%s\n", readBuf);
		fclose(fp);

		if(send(sockfd , readBuf , strlen(readBuf) , 0) < 0){
            puts("Send failed\n");
            return -1;
        }
        printf("file sended\n");

	}else{
		printf("Can't open file!\n");
	}

	return 1;
}

int main(){
	strcpy(test_pkg.filename, "../test/test.txt");
	test_pkg.timestamp = 111111; // unused
	test_pkg.partition = 2;

	init_listen_sock(LISTENING_PORT);
}