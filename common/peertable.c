#include "peertable.h"
#include "constants.h"
#include <stdlib.h> 
#include <string.h>
#include <unistd.h>
#include <stdio.h>

ts_peertable_t *tracker_peertablecreate(){
	ts_peertable_t *trackerpeertable = malloc(sizeof(ts_peertable_t));
	memset(trackerpeertable, 0, sizeof(ts_peertable_t));
	trackerpeertable->head = NULL;
	return trackerpeertable; 
}

int tracker_peertableadd(ts_peertable_t *table, tracker_peer_t *newpeer){
	newpeer->next = table->head;
	table->head = newpeer; 
	return 1;
}

int tracker_peertableremove(ts_peertable_t *table, tracker_peer_t *peer){
	tracker_peer_t *lead; 
	tracker_peer_t *trail;

	if (table->head == NULL){
		printf("Can't remove peer from empty list\n");
	}
	// Easy removal if peer is head
	if (table->head == peer){
		table->head = table->head->next; 
		free(peer);
		return 1; 
	}

	// Otherwise find peer in linked list and remove
	lead = table->head->next;
	trail = table->head; 
	while (lead != NULL && trail != NULL){
		if (lead == peer){
			trail->next = lead->next;
			free(peer);
			return 1; 
		}
		lead = lead->next;
		trail = trail->next;
	}

	printf("Peer removal failed\n");
	return -1; 
}

int tracker_peertabledestroy(ts_peertable_t *trackerpeertable){
	tracker_peer_t *temp; 
	while (trackerpeertable->head != NULL){
		temp = trackerpeertable->head;
		trackerpeertable->head = trackerpeertable->head->next;
		close(temp->sockfd);
		free(temp);
	}
	free(trackerpeertable);
	return 1; 
}

int tracker_peertableprint(ts_peertable_t *peertable){
	tracker_peer_t *peer = peertable->head;
	printf("------------------------------------CURRENT PEER TABLE--------------------------------------\n");
	while (peer != NULL){
		printf("%s\t", peer->ip);
		peer = peer->next;
	}
	printf("\n");
	// printf("left\n");
	return 0;
}