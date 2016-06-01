#include "filemonitor.h"

char* dirPath; // RELATIVE PATH TO THE DIRECTORY WE'RE MONITORING

pthread_mutex_t lock;
int listening_enabled = 1;

/*
*	INTERFACES
*/

/*
* Read a config file from a disk
* Config file stores a directory path to be monitored
* 
*/
char* readConfigFile(char* filename){
	FILE *fp;
	fp = fopen(filename, "r");
	if (fp == NULL){
		printf("Failed to open config file.\n");
		return NULL;
	}

	dirPath = (char*) malloc(sizeof(char)*128);
	memset(dirPath, 0, 128);
	char buf[128] = {0};
	memset(buf, 0, 128);
	fgets(buf, 128, fp);
	char* temp;
	temp = strtok(buf, " ");
	strcpy(dirPath, temp);
	printf("Path in config file: %s\n", dirPath);
	fclose(fp);
	return dirPath;
}

/*
* Begin to watch a directory (Actually only some initialization here)
*/
void* watchDirectory(char* directory){
	if(pthread_mutex_init(&lock, NULL) != 0){
    	printf(" mutex init failed\n");
  	}
  	return 0;
}

/*
* Get a FileInfo by giving the filename of a file
* INPUT:
* 	filename: is actuallly a path from root directory where we are monitoring 
*/
FileInfo* getFileInfo(char* filename){
	FileInfo* file;
	char fullpath[LEN_FILE_NAME];
	file = (FileInfo*) malloc(sizeof(FileInfo));
	memset(file, 0, sizeof(FileInfo));
	sprintf(fullpath, "%s/%s", dirPath, filename);
	struct stat st;
	stat(fullpath, &st);

	// COPY FILENAME, SIZE, TIMESTAMP
	strcpy(file->filepath, filename);
	if (S_ISREG(st.st_mode)){
		file->size = (long) st.st_size;
	}else{
		file->size = -1; // USE -1 TO INDICATE THE TYPE AS A DIRECTORY
	}
	
	file->lastModifyTime = (long) st.st_ctime;
	return file;
}

/*
* Thread to monitor changes in files under a directory
*/
void* monitor(void* arg){
	// printf("%s\n", "-in monitor");
	//extract information from arg
	struct filemonitorArgs *args = (struct filemonitorArgs *) arg;
	FileTable *table = args->filetable;
	pthread_mutex_t *filetable_mutex = args->filetable_mutex;
	int trackerconn = args->trackerconn;

	FileTable *newtable;
	Node* newNodePtr, *masterNodePtr, *tempPtr;
	struct timespec t;
	int found, hasChanged;
	t.tv_sec = FILE_MONITOR_INTERVAL / 1000000000;
	t.tv_nsec = FILE_MONITOR_INTERVAL - t.tv_sec*1000000000;
	// PERIODICALLY CHECKS IF THERE'S A CHANGE IN DIRECTORY
	// BY COMPARING TWO VERSIONS OF FILETABLE
	
	while (1){
		hasChanged = 0;
		
		// CHECK UPDATE
		pthread_mutex_lock(&lock);
		// if (listening_enabled){
			newtable = createTable();
			listDir(newtable, dirPath, NULL);
			
			// NOW COMPARE TWO TABLES
			masterNodePtr = table->head;
			while (masterNodePtr != NULL){
				found = 0;
				newNodePtr = newtable->head;
				while (newNodePtr != NULL){
					if (strcmp(masterNodePtr->name, newNodePtr->name) == 0){
						found = 1;
						if (masterNodePtr->timestamp == newNodePtr->timestamp){
							// SAME NAME, SAME TIMESTAMP: NO ACTION
						}
						else if (masterNodePtr->timestamp > newNodePtr->timestamp){
							// MASTER NODE HAS NEW FILE
							// MAYBE ROLLED BACK
							pthread_mutex_lock(filetable_mutex);
							modifyNode(table, newNodePtr->name, newNodePtr->size, newNodePtr->timestamp, newNodePtr->peerip[0]);
							pthread_mutex_unlock(filetable_mutex);
							hasChanged = 1;
						}
						else if (masterNodePtr->timestamp < newNodePtr->timestamp){
							// FILE UPDATED
							pthread_mutex_lock(filetable_mutex);
							modifyNode(table, newNodePtr->name, newNodePtr->size, newNodePtr->timestamp, newNodePtr->peerip[0]);
							pthread_mutex_unlock(filetable_mutex);
							hasChanged = 1;
						}
					}
					newNodePtr = newNodePtr->pNext;
				}
				if (! found){

					printf("%s deleted\n", masterNodePtr->name);
					// FILE IS DELETED
					tempPtr = masterNodePtr->pNext;
					pthread_mutex_lock(filetable_mutex);
					deleteNode(table, masterNodePtr->name);
					pthread_mutex_unlock(filetable_mutex);
					masterNodePtr = tempPtr;
					hasChanged = 1;
				}else{
					masterNodePtr = masterNodePtr->pNext;
				}
				
			}

			newNodePtr = newtable->head;
			while (newNodePtr != NULL){
				found = 0;
				masterNodePtr = table->head;
				while (masterNodePtr != NULL){
					if (strcmp(masterNodePtr->name, newNodePtr->name) == 0){
						found = 1;
						break;
					}
					masterNodePtr = masterNodePtr->pNext;
				}
				if (! found){
					printf("%s added\n", newNodePtr->name);
					// FILE IS ADDED
					pthread_mutex_lock(filetable_mutex);
					addNewNode(table, newNodePtr->name, newNodePtr->size, newNodePtr->timestamp, newNodePtr->peerip[0]);
					pthread_mutex_unlock(filetable_mutex);
					hasChanged = 1;
				}
				newNodePtr = newNodePtr->pNext;
			}
			destroyTable(newtable);
		// }
		pthread_mutex_unlock(&lock);
		///// END OF HIS IF STATEMENT
		
		if (hasChanged){
			// SEND TABLE TO TRACKER SINCE TABLE HAS MODIFIED
			printf("Table has modified: sendFileUpdate\n");
			sendFileUpdate(table, filetable_mutex, trackerconn);
		}
		nanosleep(&t, NULL);
	}
}

int sendFileUpdate(FileTable *filetable, pthread_mutex_t *filetable_mutex, int trackerconn){
  ptp_peer_t fileUpdate;
  fileUpdate.type = FILE_UPDATE; 
  memcpy(fileUpdate.peer_ip, getMyIP(), IP_LEN);
  packFileTable(filetable, filetable_mutex, fileUpdate.sendNode, &fileUpdate.file_table_size);
  if (peer_sendseg(trackerconn, &fileUpdate) < 0){
    printf("Failed to send file update packet\n");
    return -1; 
  }else{
    // printf("File update packet sent\n");
    return 1; 
  }
}

/*
*	SUPPORT FUNCTIONS
*/
//int isChanged(FileTable* prevtable, FileTable* currtable);


void blockFileListening(){
	pthread_mutex_lock(&lock);
	// listening_enabled = 0;
	// pthread_mutex_unlock(&lock);
}
void unblockFileListening(){
	// pthread_mutex_lock(&lock);
	// listening_enabled = 1;
	pthread_mutex_unlock(&lock);
}



