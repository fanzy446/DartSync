#ifndef FILETABLE_h
#define FILETABLE_h

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <linux/inotify.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>

#define MAX_PEERS 16
#define LEN_FILE_NAME 1024

/*
*	STRUCT
*/

typedef struct node{
	int size;
	char* name;
	unsigned long timestamp;
	struct node *pNext;
	int peernum;
	char** peerip;
} Node, *pNode;

typedef struct filetable{
	struct node *head;
} FileTable, filetable_t;


/*
*	INTERFACES
*/
FileTable* initTable(char* directory);
void destroyTable(FileTable*);
void addNewNode(FileTable* table, char* filename, int size, unsigned long timestamp);
int deleteNode(FileTable* table, char* filename);
int modifyNode(FileTable* table, char* filename, int size, unsigned long timestamp);


/*
*	SUPPORT FUNCTIONS
*/
FileTable* createTable();
void printTable(FileTable* table);
int getMyIP(char* ip);
Node* createNode(char* filename, int size, unsigned long timestamp);




#endif