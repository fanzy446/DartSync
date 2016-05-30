#ifndef FILEMONITOR_H 
#define FILEMONITOR_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "../common/filetable.h"
#include "../common/seg.h"
#include "../peer/p2p.h"

#define MAX_FILES 1024
#define LEN_FILE_NAME 1024
#define FILE_MONITOR_INTERVAL 100000000 // IN NANOSEC (10^-9)

typedef struct {
	char filepath[LEN_FILE_NAME];
	long size;
	long lastModifyTime;
} FileInfo;

typedef struct filemonitorArgs{
	FileTable *filetable; 
	pthread_mutex_t *filetable_mutex;
	int trackerconn;
} filemonitorArg_st; 

/*
*	INTERFACES
*/
char* readConfigFile(char* filename);
void* watchDirectory(char* directory);
FileInfo* getFileInfo(char* filename);
void* monitor(void* arg);
int sendFileUpdate(FileTable* filetable, pthread_mutex_t *filetable_mutex, int trackerconn);

/*
*	SUPPORT FUNCTIONS
*/

void blockFileListening();
void unblockFileListening();


#endif
