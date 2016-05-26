
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h> 
#include <stdlib.h> 
#include <stdio.h>
#include <netdb.h> 
#include <unistd.h>
#include "common/constants.h"
#include "common/seg.h"
#include "peer/filemonitor.h"
#include "common/filetable.h"


/***************************************************************/
//declare global variables
/***************************************************************/
int interval; 
int trackerconn; 
FileTable *table;

int peer_connToTracker();
int peer_disconnectFromTracker(int trackerconn);
void *sendheartbeat(void *arg);
int sendregister();
int seghandler();

int main(){


	//Make filetable
	// char *path = readConfigFile("./peer/config.ini");
	// table = initTable(readConfigFile(path));
	// watchDirectory(path);

	//Establish connection to tracker
	trackerconn = peer_connToTracker();
	sendregister();
	receiveTrackerState()

	pthread_t heartbeat_thread;
	pthread_create(&heartbeat_thread, NULL, sendheartbeat, (void *) 0)

	sleep(10);
	return 0;

	//sendsregister()



	// pthread_t fileMonitor_thread;
	// pthread_create(&fileMonitor_thread, NULL, monitor, (void *) table);

	while (1){
		//Listen to tracker
		
	}

	return 0; 
}

// This function connects a peer to the tracker
int peer_connToTracker(){
	int trackerconn;
	struct sockaddr_in trackeraddr;
	struct hostent *trackerInfo; 

	char hostname_buf[50];
	printf("Enter tracker hostname to connect: ");
	scanf("%s", hostname_buf);

	trackerInfo = gethostbyname(hostname_buf);
	if(!trackerInfo){
		printf("Host name error!\n");
		return -1; 
	}

	trackeraddr.sin_family = trackerInfo->h_addrtype; 
	memcpy((char *) &trackeraddr.sin_addr.s_addr, trackerInfo->h_addr_list[0], trackerInfo->h_length);
	trackeraddr.sin_port = htons(TRACKERPORT); 

	if ((trackerconn = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		printf("Socket creation failed\n");
		return -1;
	}

	if (connect(trackerconn, (struct sockaddr*)&trackeraddr, sizeof(trackeraddr))<0){
		printf("Connection to tracker failed\n");
		return -1;
	}
	printf("connection established\n");
	return trackerconn;

}

int peer_disconnectFromTracker(int trackerconn){
	close(trackerconn);
	return 1; 
}

void *sendheartbeat(void *arg){
	ptp_peer_t heartbeatseg;
	heartbeatseg.type = KEEP_ALIVE;

	while (1){
		sleep(interval);
		peer_sendseg(trackerconn, &heartbeatseg);
	}
}



int sendregister(){
	ptp_peer_t registerPacket;
	registerPacket.type = REGISTER;
	if (peer_sendseg(trackerconn, &registerPacket) < 0 ){
		printf("failed to send register packet\n");
		return 0; 
	}
	return 1; 
}

int receiveTrackerState(){
	ptp_tracker_t segment; 

	if (peer_recvseg(trackerconn, &segment) < 0){
		printf("Receive failed\n");
		return -1;
	}
	interval = segment.interval - 5;		//should give big cushion for send time
	//Now compare the files and update accordingly

}

void peer_compareFiletables(ptp_tracker_t segment){
	/////

	Node *currNode = table->head;

	for (int i = 0; i < segment.file_table_size; i++){
		currNode = table->head;
		while (currNode != NULL){
			if (strcmp(currNode->name, segment.sendNode[i].name) == 0){
				if (currNode->timestamp == segment.sendNode[i].timestamp){
					break;
				}

				else if (currNode->timestamp > segment.sendNode[i].timestamp){
					//dont send our filetable to tracker as it happens when file is updated in local and peer has sent notification but tracker haven't updated
					break;
				}
				else if (currNode->timestamp < segment.sendNode[i].timestamp){
					//update our filetable/ask to download
					modifyNode(table, currNode->name, currNode->size, currNode->timestamp);
					// block listening modify
					// download(char* filename, int size, unsigned long int timestamp, char** nodes, int numOfNodes);
					// unblock
					break;
				}

			}
		}
		if (currNode == NULL){
			addNewNode(table, currNode->name, currNode->size, currNode->timestamp);
			// block listening add
			// download(char* filename, int size, unsigned long int timestamp, char** nodes, int numOfNodes);
			// unblock
		}
	}

	currNode = table->head; 
	while (currNode != NULL){
		for (int i = 0; i < segment.file_table_size; i++){
			if (strcmp(currNode->name, segment.sendNode[i].name) == 0){
				break;
			}
		}
		deleteNode(table, currNode->name);
		// delete the actual file
		// cat DIR and currNode->name
		//remove(currNode->name);  // a stdio.h function

	}

}

