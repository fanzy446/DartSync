#include "../peer/p2p.h"
#include <stdlib.h>
#include <stdio.h> //printf
#include <string.h>    //strlen
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr

//argv[1] as the uploader
int main(int argc , char *argv[])
{
	printf("uploader:%s\n",argv[1]);
 	char* fileName = "test_large.txt";

	FILE *f = fopen(fileName,"r");
	fseek(f,0,SEEK_END);
	int size = ftell(f);
	fclose(f);

	unsigned long int timestamp = 1111111;
	char** nodes = malloc(sizeof(char*));
	if(argv[1]!=NULL){
		nodes[0] = argv[1];
	}else{
		nodes[0] = "127.0.0.1";
	}
	//nodes[0] = "127.0.0.1";

	download(fileName, size, timestamp, nodes, 1);

    return 0;
}