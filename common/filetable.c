#define _DEFAULT_SOURCE
#include <unistd.h>
#include <pthread.h>
#include "filetable.h"
#include "../common/constants.h"

FileTable* createTable(){
	FileTable* table = (FileTable*) malloc(sizeof(FileTable));
	memset(table, 0, sizeof(FileTable));
	table->head = NULL;
	return table;
}

FileTable* initTable(char* directory){
	//printf("initTable\n" );
	FileTable* table = createTable();
	listDir(table, directory, NULL);
	return table;
}

void destroyTable(FileTable* table){
	Node* curnode = table->head, *tempnode = NULL;
	while(curnode != NULL){
		tempnode = curnode->pNext;
		free(curnode);
		curnode = tempnode;
	}
	free(table);
}

void addNewNode(FileTable* table, char* filename, int size, unsigned long timestamp, char *ip){
	//printf("addNewNode for %s\n", filename);
	Node* curnode = table->head, *prevnode = NULL;
	while(curnode != NULL){
		prevnode = curnode;
		curnode = curnode->pNext;
	}
	Node* newnode = createNode(filename, size, timestamp);
	if (ip != NULL){
		memcpy(newnode->peerip, ip, IP_LEN * sizeof(char));
		newnode->peernum = 1; 
	}
	if (prevnode == NULL){
		table->head = newnode;
	}else{
		prevnode->pNext = newnode;
	}
	
}

int deleteNode(FileTable* table, char* filename){
	Node* curnode = table->head, *prevnode = NULL;
	while(curnode != NULL && strcmp(curnode->name, filename)){
		prevnode = curnode;
		curnode = curnode->pNext;
	}
	if (curnode == NULL){
		printf("no such file to be deleted\n");
		return 0;
	}
	if (prevnode == NULL){
		// target node is 1st item
		table->head = curnode->pNext;
	}else{
		prevnode->pNext = curnode->pNext;
	}
	
	free(curnode);
	return 1;
}

int modifyNode(FileTable* table, char* filename, int size, unsigned long timestamp, char *ip){
	Node* curnode = table->head;
	while(curnode != NULL && strcmp(curnode->name, filename)){
		curnode = curnode->pNext;
	}
	if (curnode == NULL){
		printf("Can't modify node- no node exists with that name\n");
		return 0;
	}
	curnode->size = size;
	curnode->timestamp = timestamp;
	if (ip != NULL){ 
		memset(curnode->peerip, 0, MAX_PEERS * IP_LEN * sizeof(char));
		memcpy(curnode->peerip, ip, IP_LEN * sizeof(char));
		curnode->peernum = 1; 
	}
	return 1;
}

/*
*	SUPPORT FUNCTIONS
*/


void printTable(FileTable* table){
	printf("------------------------------------CURRENT FILE TABLE--------------------------------------\n");
	Node* curnode = table->head;
	while(curnode != NULL){
		printf("Name:%-30s Size:%-10d Timestamp:%-15ld  ", curnode->name, curnode->size, curnode->timestamp);
		for (int i = 0; i < curnode->peernum; i++){
			printf("%s ", curnode->peerip[i]);
		}
		printf("\n");
		curnode = curnode->pNext;
	}
	printf("\n");
}

char *getMyIP(){
	char hostname_buf[50];
	if (gethostname(hostname_buf, sizeof(hostname_buf)) >= 0){
		struct hostent *he;
		if ((he = gethostbyname(hostname_buf)) == NULL){
			return 0;
		}
		struct in_addr **addr_list;
		addr_list = (struct in_addr **) he->h_addr_list;
		return inet_ntoa(*addr_list[0]);
	}
	return 0;
}

Node* createNode(char* filename, int size, unsigned long timestamp){
	Node* node = (Node*) malloc(sizeof(Node));
	node->size = size;
	strcpy(node->name, filename);
	node->timestamp = timestamp;
	node->pNext = NULL;
	return node;
}

/* This function packs the linked list contained in the FileTable into the Node array contained in segments
 */
void packFileTable(FileTable *table, pthread_mutex_t *filetable_mutex, Node nodes[], int *setNodeNum){
	pthread_mutex_lock(filetable_mutex);
	Node *currNode = table->head;

	int i = 0;
	while (currNode != NULL){
		nodes[i] = *currNode;
		currNode = currNode->pNext; 
		i++;
	}
	*setNodeNum = i; 
	pthread_mutex_unlock(filetable_mutex);
	return;
}
/* This function checks a files record to see if the Tracker knows that the
 * peer designated by ip has the file. If the peer is not already recorded as having
 * the file, it will add the peer to the list of ip's that possess it, and 
 * increment peernum.
 */

int peerHasFile(Node *fileRecord, char *ip){
	int inList = 0; 

	for (int i = 0; i < fileRecord->peernum; i++){
		if (strcmp(fileRecord->peerip[i], ip) == 0){
			inList = 1; 
			break;
		}
	}

	//If not found in list, add to list and increment peernum
	if (!inList){
		memcpy(fileRecord->peerip[fileRecord->peernum], ip, IP_LEN);
		fileRecord->peernum++;
	}
	return 1; 	
}

int removeFromFilePeers(FileTable *table, char ip[IP_LEN]){
	Node *file = table->head;
	while (file != NULL){
		for (int i = 0; i < file->peernum; i++){
			if (strcmp(file->peerip[i], ip) == 0 ){
				memmove(file->peerip[i], file->peerip[i + 1], sizeof(char) * IP_LEN * (file->peernum - i));
				memset(file->peerip[i], 0, sizeof(char) * IP_LEN);
				file->peernum--; 
				if (file->peernum == 0){
					deleteNode(table, file->name);
				}
				break; // Move on to next file
			}
		}
		file = file->pNext; 
	}
	return 1; 
}

/*
* Recursively scans all files/dirs under the directory
* , and add them as nodes into a table
*/
void listDir(FileTable* table, const char* basedir, char *location){
	DIR* d;
	char fullpath[1024];
	char dirname[1024];
	char relpath[1024];
	struct dirent *dir;
	struct stat st;

	if (location != NULL){
		sprintf(dirname, "%s/%s", basedir, location);
	}
	else{
		memcpy(dirname, basedir, sizeof(dirname));
	}


	d = opendir(dirname);
	if (d == NULL){
		printf("%s\n", strerror(errno));
	}

	if (d){
		while((dir = readdir(d)) != NULL){
			memset(fullpath, '\0', 1024);
			memset(relpath, '\0', 1024);

			if (location != NULL){
				sprintf(relpath, "%s/%s", location, dir->d_name);
			}
			else{
				memcpy(relpath, dir->d_name, sizeof(relpath));
			}
			sprintf(fullpath, "%s/%s", basedir, relpath);

			if (stat(fullpath, &st) < 0){
				printf("%s\n", "problem in stat");
			}

			// CASE: WE DON'T NEED SYMBOLIC LINK
			if (dir->d_name[0] == '.' || !strcmp(dir->d_name, ".") || !strcmp(dir->d_name, "..")){
				continue;
			}

			// CASE: REGULAR FILES
			if (S_ISREG(st.st_mode)){
				// printf("File %s: %d\n", fullpath, (int) st.st_size);
				addNewNode(table, relpath, (int) st.st_size, (unsigned long) st.st_mtime, getMyIP());
			}

			// CASE: DIRECTORIES
			if (S_ISDIR(st.st_mode)){
				//printf("Dir: %s\n", fullpath);
				addNewNode(table, relpath, -1, (unsigned long) st.st_mtime, getMyIP());
				listDir(table, basedir, relpath);
			}
		}
	}
	closedir(d);
}




