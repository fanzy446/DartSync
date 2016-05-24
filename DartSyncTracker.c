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
#include <unistd.h>
#include <sys/socket.h>
#include "common/seg.h"
#include "common/constants.h"
#include "common/peertable.h"


/***************************************************************/
//declare global variables
/***************************************************************/
ts_peertable_t *trackerpeertable;
pthread_mutex_t *trackpeertable_mutex;
//file_t *filetable;

int tracker_keepAlive(tracker_peer_t *peer);
int tracker_listenForPeers();
void* tracker_monitorAlive(void *arg);
void* tracker_Handshake(void *arg);


int main(){

	//initialize peer table as empty
	trackerpeertable = tracker_peertablecreate();

	trackpeertable_mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(trackpeertable_mutex, NULL);

	//Start listenheartbeat thread 
	pthread_t monitorAlive_thread;
	pthread_create(&monitorAlive_thread, NULL, tracker_monitorAlive, (void *) 0);

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
			newPeerEntry->last_time_stamp = 0;
			tracker_peertableadd(trackerpeertable, newPeerEntry);

			//create a new handshake for the connection
			pthread_t handshake_thread;
			pthread_create(&handshake_thread, NULL, tracker_Handshake, (void *) newPeerEntry); 
		}
	}

}

// Thread that periodically checks what peers are alive
// Removes peers from table if timeout of heartbeat
void* tracker_monitorAlive(void *arg){
	while (1){
		//get the current time 
		struct timeval currentTime;
		gettimeofday(&currentTime, NULL);

		//Iterate through all presumed to be 'alive' peers
		pthread_mutex_lock(trackpeertable_mutex);
		tracker_peer_t *peer = trackerpeertable->head; 
		while (peer != NULL){
			//remove all dead peers
			if (currentTime.tv_sec - peer->last_time_stamp > HEARTRATE && peer->last_time_stamp != 0){
				tracker_peer_t *toRemove = peer; 
				peer = peer->next;
				tracker_peertableremove(trackerpeertable, toRemove);
				continue; 
			}
			peer = peer->next;
		}
		pthread_mutex_unlock(trackpeertable_mutex);
	}



}
int tracker_keepAlive(tracker_peer_t *peer){
	//Get time
	struct timeval heartbeat; 
	gettimeofday(&heartbeat, NULL);

	//Update timestamp
	pthread_mutex_lock(trackpeertable_mutex);
	peer->last_time_stamp = heartbeat.tv_sec;
	pthread_mutex_unlock(trackpeertable_mutex); 
	return 1; 

}

// Thread that recieves messages from a specific peer and responds if needed
// there is a handshake thread for each peer
void* tracker_Handshake(void *arg){
	tracker_peer_t *peer = (tracker_peer_t *)arg; 
	ptp_peer_t segment; 


	while (1){

		//receive a segment
		if (tracker_recvseg(peer->sockfd, &segment) < 0){
			close(peer->sockfd);
			tracker_peertableremove(trackerpeertable, peer);
			pthread_exit(NULL);
		}

		//Handle depending on segment type
		switch(segment.type){
			case REGISTER: 

				//tracker sends back packet informing Interval (heartrate) and peice length for the peer to set up 
				// (tracker- acceptregister())
				break; 
			case KEEP_ALIVE: 
				tracker_keepAlive(peer);
				break; 

			case FILE_UPDATE: 
				// compare received file table with one it already has
				// if update is necessary
					//update
					// broadcast updated file table to all alive peers
				break; 
		}
	}
}

int tracker_acceptRegister(){
	return 0; 

}


