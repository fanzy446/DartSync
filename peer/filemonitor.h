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

/*
#include "../common/filetable.h"
*/

#define MAX_FILES 1024
#define LEN_FILE_NAME 1024

/*
*	STRUCT
*/

//@deprecate
typedef struct {
	char* filepath;
	long size;
	long lastModifyTime;
} FileInfo;

//@deprecate
typedef struct {
	int length;
	FileInfo** list;
} FileInfoList;


/*
*	INTERFACES
*/
void watchDirectory(char* directory);
char* readConfigFile(char* filename);
FileInfoList* getAllFilesInfo(); // need to be FileTable
void printAllFilesInfo();
void freeFileInfoList(FileInfoList*);
FileInfo* getFileInfo(char* filename);
void* monitor(void* arg);

/*
*	SUPPORT FUNCTIONS
*/
int isInFileInfoList(char* filename, FileInfoList* files);

//FileTable* createTable();
//Node* createNode(char* filename);

//extern void(*fileAdded)(char*);
//void fileAdded(char* filename);
//void fileModified(char* filename);
//void fileDeleted(char* filename);



#endif