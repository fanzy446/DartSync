#include "../peer/p2p.h"
#include <stdlib.h>
#include <stdio.h> //printf
#include <string.h>    //strlen
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr

int main(int argc , char *argv[])
{
 	char* fileName = "test_large.txt";

	// FILE *f = fopen(fileName,"rb");
	// fseek(f,0,SEEK_END);
	// int size = ftell(f);
	// fclose(f);

	unsigned long int timestamp = 1111111;
	char nodes[][IP_LEN] = {"129.170.212.87"};

	download(fileName, size, timestamp, nodes, 1);

    return 0;
}