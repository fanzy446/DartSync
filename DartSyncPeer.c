
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h> 
#include <stdlib.h> 
#include <stdio.h>
#include <netdb.h> 
#include <unistd.h>
#include "common/constants.h"


/***************************************************************/
//declare global variables
/***************************************************************/
int interval; 
int trackerconn; 

int main(){

	//Establish connection to tracker
	trackerconn = peer_connToTracker();

	//send register

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

	return trackerconn;

}

int peer_disconnectFromTracker(int trackerconn){
	close(trackerconn);
	return 1; 
}

void *sendheartbeat(void *arg){
	// create segment
	ptp_peer_t heartbeatseg;
	heartbeatseg.type = KEEP_ALIVE;

	while (1){
		sleep(interval);
		peer_sendseg(trackerconn, segment);
	}
	return NULL;
}

