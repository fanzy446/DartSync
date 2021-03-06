#include "../peer/p2p.h"
#include <stdlib.h>
#include <stdio.h> //printf
#include <string.h>    //strlen
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr

//download 3 files from 3 upload peers

//whaleback.cs.dartmouth.edu (129.170.212.87) or your localhost as downloader
//flume.cs.dartmouth.edu (129.170.214.115) uploader
//stowe.cs.dartmouth.edu (129.170.213.62) uploader
//carter.cs.dartmouth.edu (129.170.214.100) uploader


int getFileSize(char* fileName){
	FILE *f = fopen(fileName,"rb");
	fseek(f,0,SEEK_END);
	int size = ftell(f);
	fclose(f);

	return size;
}

int main(int argc , char *argv[])
{
 	char* large_fileName = "test_large.txt";
 	char* short_fileName1 = "haha.jpg";
 	char* short_fileName2 = "test_short_2.txt";

	int size1 = getFileSize(large_fileName);
	int size2 = getFileSize(short_fileName1);
	int size3 = getFileSize(short_fileName2);

	unsigned long int timestamp = 1111111;
	char nodes[][IP_LEN] = {"129.170.214.115", "129.170.213.62", "129.170.214.100"};

	char* root = "droot";
	download(root, large_fileName, size1, timestamp, nodes, 3);
	download(root, short_fileName1, size2, timestamp, nodes, 3);
	download(root, short_fileName2, size3, timestamp, nodes, 3);

    return 0;
}