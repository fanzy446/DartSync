/* FILE: Seg.c
 */

#include "seg.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>


//tracker function to send segment to peer
int tracker_sendseg(int peerconn, ptp_tracker_t *segment)
{

	char bufstart[2];
	char bufend[2];
	bufstart[0] = '!';
	bufstart[1] = '&';
	bufend[0] = '!';
	bufend[1] = '#';
	printf("buf");
	if (send(peerconn, bufstart, 2, 0) < 0) {
		return -1;
	}
	printf("bufsent\n");
	if(send(peerconn, segment,sizeof(ptp_tracker_t),0)<0) {
		return -1;
	}
	printf("stuck\n");
	if(send(peerconn,bufend,2,0)<0) {
		return -1;
	}
	printf("what\n");
	fflush(stdout);
	return 1;
}

int tracker_recvseg(int peerconn, ptp_peer_t *segment)
{
	ptp_peer_t receiveseg;
	char buf[sizeof(ptp_peer_t)+2]; 
	char c;
	int idx = 0;
	// state can be 0,1,2,3; 
	// 0 starting point 
	// 1 '!' received
	// 2 '&' received, start receiving segment
	// 3 '!' received,
	// 4 '#' received, finish receiving segment 
	int state = 0; 
	while(recv(peerconn,&c,1,0)>0) {
		if (state == 0) {
		        if(c=='!')
				state = 1;
		}
		else if(state == 1) {
			if(c=='&') 
				state = 2;
			else
				state = 0;
		}
		else if(state == 2) {
			if(c=='!') {
				buf[idx]=c;
				idx++;
				state = 3;
			}
			else {
				buf[idx]=c;
				idx++;
			}
		}
		else if(state == 3) {
			if(c=='#') {
				buf[idx]=c;
				idx++;
				state = 0;
				idx = 0;
				//add segment error	
				memcpy(&receiveseg,buf,sizeof(ptp_peer_t));
				*segment = receiveseg;
				return 1;
			}
			else if(c=='!') {
				buf[idx]=c;
				idx++;
			}
			else {
				buf[idx]=c;
				idx++;
				state = 2;
			}
		}
	}
	return -1;
}

//Peer function to send segment to tracker
int peer_sendseg(int trackerconn, ptp_peer_t *segment)
{

	char bufstart[2];
	char bufend[2];
	bufstart[0] = '!';
	bufstart[1] = '&';
	bufend[0] = '!';
	bufend[1] = '#';
	if (send(trackerconn, bufstart, 2, 0) < 0) {
		return -1;
	}
	if(send(trackerconn,segment,sizeof(ptp_peer_t),0)<0) {
		return -1;
	}
	if(send(trackerconn,bufend,2,0)<0) {
		return -1;
	}
	return 1;
}

int peer_recvseg(int trackerconn, ptp_tracker_t *segment)
{
	ptp_tracker_t receiveseg;
	char buf[sizeof(ptp_tracker_t)+2]; 
	char c;
	int idx = 0;
	// state can be 0,1,2,3; 
	// 0 starting point 
	// 1 '!' received
	// 2 '&' received, start receiving segment
	// 3 '!' received,
	// 4 '#' received, finish receiving segment 
	int state = 0; 
	while(recv(trackerconn,&c,1,0)>0) {
		if (state == 0) {
		        if(c=='!')
				state = 1;
		}
		else if(state == 1) {
			if(c=='&') 
				state = 2;
			else
				state = 0;
		}
		else if(state == 2) {
			if(c=='!') {
				buf[idx]=c;
				idx++;
				state = 3;
			}
			else {
				buf[idx]=c;
				idx++;
			}
		}
		else if(state == 3) {
			if(c=='#') {
				buf[idx]=c;
				idx++;
				state = 0;
				idx = 0;
				//add segment error	
				memcpy(&receiveseg,buf,sizeof(ptp_tracker_t));
				*segment = receiveseg;
				return 1;
			}
			else if(c=='!') {
				buf[idx]=c;
				idx++;
			}
			else {
				buf[idx]=c;
				idx++;
				state = 2;
			}
		}
	}
	return -1;
}


