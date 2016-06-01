
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h> 
#include <stdlib.h> 
#include <stdio.h>
#include <signal.h>
#include <netdb.h> 
#include <unistd.h>
#include <dirent.h>
#include "common/constants.h"
#include "common/seg.h"
#include "peer/filemonitor.h"
#include "common/filetable.h"
#include "peer/p2p.h"


/***************************************************************/
//declare global variables
/***************************************************************/
int interval; 
int trackerconn; 
FileTable *filetable;
char *path;
pthread_mutex_t *filetable_mutex; 

int peer_connToTracker();
int peer_disconnectFromTracker(int trackerconn);
void *sendheartbeat(void *arg);
int sendregister();
int seghandler();
int receiveTrackerState(int firstContact);
int peer_compareFiletables(ptp_tracker_t segment, int firstContact);
int listenToTracker();
void peer_stop();
int remove_directory_recursively(char *path);


int main(){

	//register a signal handler which is used to terminate the process
	signal(SIGINT, peer_stop);

	//Make filetable and print it
	path = readConfigFile("./config.ini");
	filetable = initTable(path);
	watchDirectory(path);
	printTable(filetable);

	//create mutex for filetable
	filetable_mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(filetable_mutex, NULL);

	//Create thread to listen to peers asking for downloads
	pthread_t peer2peer_thread;
	pthread_create(&peer2peer_thread, NULL, start_listening, (void *) path);

	//Establish connection to tracker
	trackerconn = peer_connToTracker();
	sendregister();
	if (receiveTrackerState(1) > 0){
		sendFileUpdate(filetable, filetable_mutex, trackerconn);
		// printTable(filetable);
	}

	//Start thread to let tracker know it is still alive
	int *iptr = malloc(sizeof(int));
	*iptr = interval; 
	pthread_t heartbeat_thread;
	pthread_create(&heartbeat_thread, NULL, sendheartbeat, iptr);

	// Create filemonitor thread and its args
	filemonitorArg_st *args = malloc(sizeof(filemonitorArg_st));
	args->filetable = filetable;
	args->filetable_mutex = filetable_mutex;
	args->trackerconn = trackerconn;
	pthread_t fileMonitor_thread;
	pthread_create(&fileMonitor_thread, NULL, monitor, (void *) args);


	while (trackerconn != -100){				// While the connection to tracker still exists
		if (receiveTrackerState(0) > 0){
			sendFileUpdate(filetable, filetable_mutex, trackerconn);
			printTable(filetable);
		}
		// receiveTrackerState(0);
		// sendFileUpdate(filetable, filetable_mutex, trackerconn);
	}
	exit(0);
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
	printf("Connected to tracker\n");
	return trackerconn;

}

void peer_stop(){
	printf("Received SIGINT\n");
	close(trackerconn);
	trackerconn = -1; 
	destroyTable(filetable);
	free(filetable_mutex);
	exit(0); 
}

void *sendheartbeat(void *arg){
	int heartrate = *((int *) arg); 
	while (1){
		ptp_peer_t heartbeatseg;
		heartbeatseg.type = KEEP_ALIVE;
		if (peer_sendseg(trackerconn, &heartbeatseg) < 0) {
			printf("Failed to send heartbeat\n");
			return NULL; 
		}
		sleep(heartrate);
		fflush(stdout);
	}
}


int sendregister(){
	ptp_peer_t registerPacket;
	memset(&registerPacket, 0, sizeof(ptp_peer_t));
	registerPacket.type = REGISTER;
	memcpy(registerPacket.peer_ip, getMyIP(), IP_LEN);
	if (peer_sendseg(trackerconn, &registerPacket) < 0 ){
		printf("failed to send register packet\n");
		return 0; 
	}
	printf("Register packet sent\n");
	return 1; 
}


int receiveTrackerState(int firstContact){
	ptp_tracker_t segment; 
	int update;
	memset(&segment, 0, sizeof(ptp_tracker_t));

	if (peer_recvseg(trackerconn, &segment) < 0){
		printf("Receive failed\n");
		trackerconn = -100; 
		return -1;
	}
	printf("\n-------------------Global File State-------------------\n");
	interval = segment.interval - 5;		//should give big cushion for send time
	// printf("Received global file state\n");
	//Now compare the files and update accordingly
	for (int i = 0 ; i < segment.file_table_size; i++){
		printf("%s %d %ld\n", segment.sendNode[i].name, segment.sendNode[i].size, segment.sendNode[i].timestamp);
	}
	printf("\n");
	pthread_mutex_lock(filetable_mutex);
	update = peer_compareFiletables(segment, firstContact);
	pthread_mutex_unlock(filetable_mutex);
	return update; 

}


int peer_compareFiletables(ptp_tracker_t segment, int firstContact){
	
	blockFileListening();
	Node *currNode;
	int i; 
	int sendUpdate;
	int found[segment.file_table_size];
	memset(found, 0, segment.file_table_size * sizeof(int));

	sendUpdate = 0; 
	currNode = filetable->head;

	while (currNode != NULL){
		for (i = 0; i < segment.file_table_size; i++){
			if (strcmp(currNode->name, segment.sendNode[i].name) == 0){
				found[i] = 1; 
				if (currNode->timestamp == segment.sendNode[i].timestamp){
					break;
				}
				else if (currNode->timestamp > segment.sendNode[i].timestamp){
					sendUpdate = 1; 

					break;
				}
				else if (currNode->timestamp < segment.sendNode[i].timestamp){
					modifyNode(filetable, segment.sendNode[i].name, segment.sendNode[i].size, segment.sendNode[i].timestamp, NULL);
					sendUpdate = 1; 

					//block listening
					if( segment.sendNode[i].size == -1){
						char* dir = getPath(path, segment.sendNode[i].name);
						struct utimbuf fakeTime;
						fakeTime.actime = segment.sendNode[i].timestamp;
						fakeTime.modtime = segment.sendNode[i].timestamp;
						if(utime(dir, &fakeTime) < 0){
							perror("change Directory meta data failed.");
						}
					}else{
						download(path, segment.sendNode[i].name, segment.sendNode[i].size, segment.sendNode[i].timestamp, segment.sendNode[i].peerip, segment.sendNode[i].peernum);
					}
					break; 
				}
			}
		}

		//If have file that tracker isn't tracking..
		if (i == segment.file_table_size){
			printf("Peer has file %s that tracker does not\n", currNode->name);
			sendUpdate = 1; 
			if (firstContact == 1){
			} else{
				
				char* filepath = getPath(path, currNode->name);
				printf("Peer remove %s\n", filepath);
				if(currNode->size == -1){
					remove_directory_recursively(filepath);
				}else{
					remove(filepath);
				}
				free(filepath);
				deleteNode(filetable, currNode->name);
			}
		} 
		currNode = currNode->pNext;

	}

	//Iterate through found to find tracker files that were not found in local peer
	for (int i = 0; i < segment.file_table_size; i++){
		if (found[i] == 0){
			printf("Peer doesn't have file %s that tracker has\n", segment.sendNode[i].name);

			//Add node to the mutex corresponding to the file
			addNewNode(filetable, segment.sendNode[i].name, segment.sendNode[i].size, segment.sendNode[i].timestamp, NULL);
			sendUpdate = 1; 

			if (segment.sendNode[i].size == -1){
				char* dir = getPath(path, segment.sendNode[i].name);
				mkdir(dir, S_IRWXU);
				struct utimbuf fakeTime;
				fakeTime.actime = segment.sendNode[i].timestamp;
				fakeTime.modtime = segment.sendNode[i].timestamp;
				if(utime(dir, &fakeTime) < 0){
					perror("change Directory meta data failed.");
				}
			}
			else{
				// printf("Filename to download: %s\n", segment.sendNode[i].name); 
				download(path, segment.sendNode[i].name, segment.sendNode[i].size, segment.sendNode[i].timestamp, segment.sendNode[i].peerip, segment.sendNode[i].peernum);
			}
		}
	}

	unblockFileListening();

	return sendUpdate;
}

int remove_directory_recursively(char *path) {
   DIR *d = opendir(path);
   if(d){
   		struct dirent *child;
   		while((child = readdir(d))){
			if (!strcmp(child->d_name, ".") || !strcmp(child->d_name, "..")) {
	            continue;
	        }
	        char *buf = getPath(path, child->d_name);
	        struct stat statbuf;
	        stat(buf, &statbuf);
            if (S_ISDIR(statbuf.st_mode)){
               if(remove_directory_recursively(buf) < 0){
               		return -1;
               }
            } else if(unlink(buf) < 0){
               	return -1;
            }
            free(buf);
   		}
   		closedir(d);
   }
   rmdir(path);
   return 1;
}


