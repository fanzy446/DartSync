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

#include "../common/constants.h"
/*
*	STRUCT
*/

typedef struct node{
	int size;
	char name[LEN_FILE_NAME];
	unsigned long timestamp;
	struct node *pNext;
	int peernum;
	char peerip[MAX_PEERS][IP_LEN];
} Node, *pNode;

typedef struct filetable{
	int numNodes; 
	struct node *head;
} FileTable, filetable_t;







/*
*	INTERFACES
*/
FileTable* initTable(char* directory);
void destroyTable(FileTable*);
void addNewNode(FileTable* table, char* filename, int size, unsigned long timestamp, char *ip);
int deleteNode(FileTable* table, char* filename);
int modifyNode(FileTable* table, char* filename, int size, unsigned long timestamp, char *ip);
void packFileTable(FileTable *table, pthread_mutex_t *filetable_mutex, Node nodes[], int *setNodeNum);
int peerHasFile(Node *fileRecord, char *ip);


/*
*	SUPPORT FUNCTIONS
*/
FileTable* createTable();
void printTable(FileTable* table);
char* getMyIP();
Node* createNode(char* filename, int size, unsigned long timestamp);


#endif