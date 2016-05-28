//FILE: common/peertable.h 
//
//Description: 
//
//Date: May 18, 2016


#ifndef PEERTABLE_H 
#define PEERTABLE_H

#include "constants.h"

//Peer-side peer table.
typedef struct _peer_side_peer_t {
    // Remote peer IP address, 16 bytes.
    char ip[IP_LEN];
    //Current downloading file name.
    char file_name[FILE_NAME_LEN];
    //Timestamp of current downloading file.
    unsigned long file_time_stamp;
    //TCP connection to this remote peer.
    int sockfd;
    //Pointer to the next peer, linked list.
    struct _peer_side_peer_t *next;
} peer_peer_t;


//Tracker-side peer table
typedef struct _tracker_side_peer_t {
    //Remote peer IP address, 16 bytes.
    char ip[IP_LEN];
    //Last alive timestamp of this peer.
    unsigned long last_time_stamp;
    //TCP connection to this remote peer.
    int sockfd;
    //Pointer to the next peer, linked list.
    struct _tracker_side_peer_t *next;
} tracker_peer_t;

typedef struct ts_peertable{
    struct _tracker_side_peer_t *head;
}ts_peertable_t;

ts_peertable_t *tracker_peertablecreate();
int tracker_peertableadd(ts_peertable_t *table, tracker_peer_t *newpeer);
int tracker_peertableremove(ts_peertable_t *table, tracker_peer_t *peer);
int tracker_peertabledestroy(ts_peertable_t *trackerpeertable);
int tracker_peertableprint(ts_peertable_t *peertable);

#endif