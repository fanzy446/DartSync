#ifndef FILEMONITOR_H 
#define FILEMONITOR_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#include "../common/filetable.h"
#include "../common/seg.h"
#include "../peer/p2p.h"


#define MAX_FILES 1024
#define LEN_FILE_NAME 1024

/*
*	STRUCT
*/

typedef struct {
	char* filepath;
	long size;
	long lastModifyTime;
} FileInfo;

typedef struct {
	int length;
	FileInfo** list;
} FileInfoList;


typedef struct filemonitorArgs{
	FileTable *filetable; 
	pthread_mutex_t *filetable_mutex;
	int trackerconn;
} filemonitorArg_st; 

/*
*	INTERFACES
*/
void watchDirectory(char* directory);
char* readConfigFile(char* filename);
FileInfoList* getAllFilesInfo();
void printAllFilesInfo();
void freeFileInfoList(FileInfoList*);
FileInfo* getFileInfo(char* filename);
void* monitor(void* arg);
int sendFileUpdate(FileTable *filetable, pthread_mutex_t *filetable_mutex, int trackerconn); 
void sendTable(FileTable* table);

/*
*	SUPPORT FUNCTIONS
*/
int isInFileInfoList(char* filename, FileInfoList* files);
void fileAdded(FileTable* table, char* filename, pthread_mutex_t *filetable_mutex, int trackerconn);
void fileModified(FileTable* table, char* filename, pthread_mutex_t *filetable_mutex, int trackerconn);
void fileDeleted(FileTable* table, char* filename, pthread_mutex_t *filetable_mutex, int trackerconn);

void blockFileAddListening();
void unblockFileAddListening();
void blockFileWriteListening();
void unblockFileWriteListening();
void blockFileDeleteListening();
void unblockFileDeleteListening();



#endif