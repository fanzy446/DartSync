#include "filetable.h"

FileTable* createTable(){
	FileTable* table = (FileTable*) malloc(sizeof(FileTable));
	table->head = NULL;
	return table;
}

void destroyTable(FileTable* table){
	Node* curnode = table->head, *tempnode;
	int i;
	while(curnode != NULL){
		free(curnode->name);
		for (i=0; i<curnode->peernum; i++){
			free(curnode->peerip[i]);
		}
		free(curnode->peerip);
		tempnode = curnode->pNext;
		free(curnode);
		curnode = tempnode;
	}
	free(table);
}


void addNewNode(FileTable* table, char* filename, int size, unsigned long timestamp){
	Node* curnode = table->head, *prevnode;
	while(curnode != NULL){
		prevnode = curnode;
		curnode = curnode->pNext;
	}
	Node* newnode = createNode(filename, size, timestamp);
	prevnode->pNext = newnode;
}

int deleteNode(FileTable* table, char* filename){
	Node* curnode = table->head, *prevnode;
	while(curnode != NULL && !strcmp(curnode->name, filename)){
		prevnode = curnode;
		curnode = curnode->pNext;
	}
	if (curnode == NULL){
		return 0;
	}
	prevnode->pNext = curnode->pNext;
	free(curnode->name);
	int i;
	for (i=0; i<curnode->peernum; i++){
		free(curnode->peerip[i]);
	}
	free(curnode->peerip);
	free(curnode);
	return 1;
}

int modifyNode(FileTable* table, char* filename, int size, unsigned long timestamp){
	Node* curnode = table->head;
	while(curnode != NULL && !strcmp(curnode->name, filename)){
		curnode = curnode->pNext;
	}
	if (curnode == NULL){
		return 0;
	}
	curnode->size = size;
	curnode->timestamp = timestamp;
	return 1;
}

/*
*	SUPPORT FUNCTIONS
*/

int getMyIP(char* ip){
	char hostname_buf[50];
	if (gethostname(hostname_buf, sizeof(hostname_buf)) >= 0){
		struct hostent *he;
		if ((he = gethostbyname(hostname_buf)) == NULL){
			return 0;
		}
		struct in_addr **addr_list;
		addr_list = (struct in_addr **) he->h_addr_list;
		strcpy(ip, inet_ntoa(*addr_list[0]));
		return 1;
	}
	return 0;
}

Node* createNode(char* filename, int size, unsigned long timestamp){
	Node* node = (Node*) malloc(sizeof(Node));
	node->size = size;
	node->name = filename;
	node->timestamp = timestamp;
	node->pNext = NULL;
	node->peernum = 1;
	node->peerip = (char**)malloc(sizeof(char*)*MAX_PEERS);
	node->peerip[0] = (char*)malloc(sizeof(char)*100);
	getMyIP(node->peerip[0]);
	return node;
}

/*
int main(){
	char* ip = (char*)malloc(sizeof(char)*100);
	getMyIP(ip);
	printf("%s\n", ip);

	FileTable* table = createTable();

}
*/