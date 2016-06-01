/* FILE: DartSyncTracker.c
 * Description: 
 *
 * Date: May 23, 2016
 *
 * 
 */
#include <stdlib.h>
#include <stdio.h> 
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <arpa/inet.h> 
#include <signal.h> 
#include <unistd.h>
#include <sys/socket.h>
#include "common/seg.h"
#include "common/constants.h"
#include "common/peertable.h"

#define ALIVE 1 
#define DEAD 0
/***************************************************************/
//declare global variables
/***************************************************************/
ts_peertable_t *peertable;
FileTable *filetable;
pthread_mutex_t *peertable_mutex;
pthread_mutex_t *filetable_mutex;


int tracker_keepAlive(tracker_peer_t *peer);
int tracker_listenForPeers();
void* tracker_monitorAlive(void *arg);
void* tracker_Handshake(void *arg);
int tracker_acceptRegister();
int tracker_compareFiletables(ptp_peer_t segment);
int broadcastUpdates();
void *connWeb(void *arg); 
void tracker_stop();


int main(){

	//register a signal handler which is used to terminate the process
	signal(SIGINT, tracker_stop);

	//initialize peer and file table as empty
	peertable = tracker_peertablecreate();
	filetable = createTable();

	//create mutexes for peertable and filetable
	peertable_mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(peertable_mutex, NULL);
	filetable_mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(filetable_mutex, NULL);

	//Start webapp connection thread
	printf("Starting web thread\n");
	pthread_t webconn_thread;
	pthread_create(&webconn_thread, NULL, connWeb, (void*) 0);

	//Start listenheartbeat thread 
	printf("Starting monitor alive thread\n");
	pthread_t monitorAlive_thread;
	pthread_create(&monitorAlive_thread, NULL, tracker_monitorAlive, (void *) 0);

	//Add peer connections and create handshake threads for each of them
	tracker_listenForPeers();

}

int tracker_listenForPeers(){
	int listensd;
	int newpeer;
	struct sockaddr_in listen_addr; 
	struct sockaddr_in peer_addr;
	socklen_t peer_addr_len;

	if ((listensd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
		printf("socket creation failed\n");
		return -1;
	}

	memset(&listen_addr, 0, sizeof(listen_addr));
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	listen_addr.sin_port = htons(TRACKERPORT);

	peer_addr_len = sizeof(peer_addr);

	if (bind(listensd, (struct sockaddr *)&listen_addr, sizeof(listen_addr)) < 0){
		printf("Tracker listensd bind failed\n");
		return -1; 
	}

	if (listen(listensd, 20) < 0){
		printf("Tracker listen failed\n");
		return -1;
	}

	while (1){
		if ((newpeer = accept(listensd, (struct sockaddr*) &peer_addr, &peer_addr_len)) != -1){
			printf("New peer connected\n");
			//create a new peer entry
			tracker_peer_t *newPeerEntry = malloc(sizeof(tracker_peer_t));
			newPeerEntry->sockfd = newpeer;
			newPeerEntry->next = NULL; 
			memcpy(newPeerEntry->ip, inet_ntoa(peer_addr.sin_addr), IP_LEN);
			newPeerEntry->last_time_stamp = -10;										//Hasn't received the interval yet
			tracker_peertableadd(peertable, newPeerEntry);

			//create a new handshake for the connection
			pthread_t handshake_thread;
			pthread_create(&handshake_thread, NULL, tracker_Handshake, (void *) newPeerEntry); 
		}
	}

}

void tracker_stop(){
	printf("Received SIGINT\n");
	destroyTable(filetable);
	tracker_peertabledestroy(peertable);
	pthread_mutex_destroy(peertable_mutex);
	pthread_mutex_destroy(filetable_mutex); 
	exit(0);
}

void* tracker_monitorAlive(void *arg){
	while (1){
		tracker_peer_t *toRemove;
		tracker_peer_t *peer; 
		struct timeval currentTime; 
		gettimeofday(&currentTime, NULL);

		pthread_mutex_lock(peertable_mutex);
		peer = peertable->head; 
		while (peer != NULL){
			if ((currentTime.tv_sec - peer->last_time_stamp > HEARTRATE) && (peer->last_time_stamp != -10)){
				printf("Peer disconnected\n");
				toRemove = peer;
				peer = peer->next; 
				pthread_mutex_lock(filetable_mutex);
				removeFromFilePeers(filetable, toRemove->ip);
				pthread_mutex_unlock(filetable_mutex);
				tracker_peertableremove(peertable, toRemove);
				printTable(filetable);
				continue;
			}
			peer = peer->next; 
		}

		pthread_mutex_unlock(peertable_mutex);



	}
}

int tracker_keepAlive(tracker_peer_t *peer){
	//Get time
	struct timeval heartbeat; 
	gettimeofday(&heartbeat, NULL);

	//Update timestamp
	pthread_mutex_lock(peertable_mutex);
	peer->last_time_stamp = heartbeat.tv_sec;
	pthread_mutex_unlock(peertable_mutex); 
	return 1; 

}

void *connWeb(void *arg){
	int listensd;
	int webconn;
	Node *currNode; 
	struct sockaddr_in listen_addr; 
	struct sockaddr_in webconn_addr;
	socklen_t webconn_addr_len;

	if ((listensd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
		printf("socket creation failed\n");
		return NULL;
	}

	memset(&listen_addr, 0, sizeof(listen_addr));
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	listen_addr.sin_port = htons(WEB_PORT);

	webconn_addr_len = sizeof(webconn_addr);

	if (bind(listensd, (struct sockaddr *)&listen_addr, sizeof(listen_addr)) < 0){
		printf("Tracker listensd bind failed\n");
		return NULL;
	}

	if (listen(listensd, 20) < 0){
		printf("Tracker listen failed\n");
		return NULL;
	}

	while (1){
		if ((webconn = accept(listensd, (struct sockaddr*) &webconn_addr, &webconn_addr_len)) != -1){
			while (1){
				char filetablebuf[10000]; 
				char nodebuf[300];
				memset(filetablebuf, 0, sizeof(filetablebuf));
				
				pthread_mutex_lock(filetable_mutex);
				currNode = filetable->head; 
				while (currNode != NULL){
					memset(nodebuf, 0, sizeof(nodebuf));
					sprintf(nodebuf,"%s %d %ld|", currNode->name, currNode->size, currNode->timestamp);
					strcat(filetablebuf, nodebuf);
					currNode = currNode->pNext;
				}
				pthread_mutex_unlock(filetable_mutex);
				send(webconn, filetablebuf, sizeof(filetablebuf), 0);
				sleep(3);
			}	
		}
	}
}


// Thread that recieves messages from a specific peer and responds if needed
// there is a handshake thread for each peer
void* tracker_Handshake(void *arg){
	tracker_peer_t *peer = (tracker_peer_t *)arg; 
	ptp_peer_t segment; 


	while (1){

		//receive a segment
		if (tracker_recvseg(peer->sockfd, &segment) < 0){
			pthread_exit(NULL);
		}

		//Handle depending on segment type
		switch(segment.type){
			case REGISTER: 
				printf("recieved register\n");
				tracker_acceptRegister(peer->sockfd);
				break; 
			case KEEP_ALIVE: 
				tracker_keepAlive(peer);
				printf("Received keep alive message\n");
				break; 
			case FILE_UPDATE: 
				printf("received file update packet\n");
				fflush(stdout);
				if (tracker_compareFiletables(segment) > 0){
					broadcastUpdates();
					printTable(filetable);
				}
				break; 
		}
	}
}

/* This function responds to a REGISTER packet by giving the newly connected peer the currentfilestate
 * as well as the HEARTRATE (interval) for the peer to set up
 */
int tracker_acceptRegister(int peerconn){
	ptp_tracker_t segment;
	segment.interval = HEARTRATE;
	packFileTable(filetable, filetable_mutex, segment.sendNode, &segment.file_table_size);

	if (tracker_sendseg(peerconn, &segment)){
		return 1;
		printf("Sent table");
	}
	else{
		return -1; 
		printf("failed to send table");
	}
}


//needs to take in the segments file table or the segment itself
int tracker_compareFiletables(ptp_peer_t segment){
	Node *currNode; 
	int i, broadcast;
	int havePeerFile[segment.file_table_size];
	memset(havePeerFile, 0, segment.file_table_size *sizeof(int));

	broadcast = 0; 
	currNode = filetable->head;
	while (currNode != NULL){
		for (i = 0; i < segment.file_table_size; i++){
			if (strcmp(currNode->name, segment.sendNode[i].name) == 0){
				havePeerFile[i] = 1; 

				if (currNode->timestamp == segment.sendNode[i].timestamp){
					peerHasFile(currNode, segment.peer_ip);	
					broadcast = 1; 					//Checks to make sure tracker has record of peer possessing this file
					break;
				}
				else if (currNode->timestamp > segment.sendNode[i].timestamp){
					printf("Tracker has more recent version of file '%s'\n", currNode->name); 
					break;
				}
				else if (currNode->timestamp < segment.sendNode[i].timestamp){
					printf("Peer has more recent version of '%s', telling other peers...\n", currNode->name);
					broadcast = 1; 
					pthread_mutex_lock(filetable_mutex);
					modifyNode(filetable, segment.sendNode[i].name, segment.sendNode[i].size, segment.sendNode[i].timestamp, segment.peer_ip);
					pthread_mutex_unlock(filetable_mutex);
					break;
				}
			}
		}
		// If has iterated through all incoming segments with no match
		// Peer has deleted a file -> remove this file from filetable
		if (i == segment.file_table_size){		
			printf("Peer deleted file... '%s' tracker removing from filetable\n", currNode->name);
			broadcast = 1; 
			pthread_mutex_lock(filetable_mutex);
			deleteNode(filetable, currNode->name);
			pthread_mutex_unlock(filetable_mutex);
		}

		currNode = currNode->pNext; 
	}
	for (int i = 0; i < segment.file_table_size; i++){
		if (!havePeerFile[i]){
			printf("Peer has added file named '%s\n", segment.sendNode[i].name);
			broadcast = 1; 
			pthread_mutex_lock(filetable_mutex);
			addNewNode(filetable, segment.sendNode[i].name, segment.sendNode[i].size, segment.sendNode[i].timestamp, segment.peer_ip); 
			pthread_mutex_unlock(filetable_mutex);
		}
	}
	return broadcast;
}

int broadcastUpdates(){
	tracker_peer_t *currPeer;
	ptp_tracker_t broadcast;

	//Construct broadcast message
	packFileTable(filetable, filetable_mutex, broadcast.sendNode, &broadcast.file_table_size);

	//Send to all alive peers
	pthread_mutex_lock(peertable_mutex);
	currPeer = peertable->head;
	while (currPeer != NULL){
		if (tracker_sendseg(currPeer->sockfd, &broadcast) < 0 ){
			printf("send broadcast failed at peer %d\n", currPeer->sockfd);
			pthread_mutex_unlock(peertable_mutex);
			return -1;
		}
		printf("Broadcast filetable\n");
		currPeer = currPeer->next; 
	}
	pthread_mutex_unlock(peertable_mutex);
	return 1; 
}


