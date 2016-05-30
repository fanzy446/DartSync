#include "../../peer/p2p.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <arpa/inet.h>

int main(int argc , char *argv[]){
	int listenfd, connfd, n;
	struct sockaddr_in cli_addr, serv_addr;
	socklen_t clilen;

	if ((listenfd = socket (AF_INET, SOCK_STREAM, 0)) <0)
	{
		perror("Error when creating listen socket");
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(1234);
	bind(listenfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
	listen (listenfd, 10);
	while(1){
		clilen = sizeof(char)*10;
		connfd = accept(listenfd, (struct sockaddr *)&cli_addr, (socklen_t*)&clilen);
	    if (connfd < 0) {
	        perror("accept failed");
	        break;
	    }
	    printf("Connection accepted\n");
		printf("from ip:%s | port:%d ", inet_ntoa(cli_addr.sin_addr), cli_addr.sin_port);
		while(1){
			if (send(connfd, "{\"data1\":\"helloworld\"}", 22, 0) < 0) {
	        	perror("download_sendpkt: send start failed");
	        	return -1;
	    	}
			sleep(3);
    	}
	}
	close(listenfd);
	return -1;
}