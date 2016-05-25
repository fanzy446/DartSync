#ifndef FILEMONITOR_H 
#define FILEMONITOR_H

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

#include "../common/filetable.h"
#include "../common/seg.h"
//#include "../common/p2p.h"


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
void setTrackerConn(int conn);
void sendTable(FileTable* table);

/*
*	SUPPORT FUNCTIONS
*/
int isInFileInfoList(char* filename, FileInfoList* files);
void fileAdded(FileTable* table, char* filename);
void fileModified(FileTable* table, char* filename);
void fileDeleted(FileTable* table, char* filename);



#endif