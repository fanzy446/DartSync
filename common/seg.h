

#ifndef SEG_H 
#define SEG_H

#define REGISTER 0
#define KEEP_ALIVE 1
#define FILE_UPDATE 2

#include "../common/constants.h"


/* The packet data structure sending from peer to tracker */
typedef struct segment_peer {
  // protocol length
  int protocol_len;
  // protocol name
  char protocol_name[PROTOCOL_LEN + 1];
  // packet type : register, keep alive, update file table
  int type;
  // reserved space, you could use this space for your convenient, 8 bytes by default
  char reserved[RESERVED_LEN];
  // the peer ip address sending this packet
  char peer_ip[IP_LEN];
  // listening port number in p2p
  int port;
  // the number of files in the local file table -- optional
  int file_table_size;
  // file table of the client -- your own design
  //file_t file_table;
}ptp_peer_t;



/* The packet data structure sending from tracker to peer */
typedef struct segment_tracker{
// time interval that the peer should sending alive message periodically
int interval;
// piece length
int piece_len;
// file number in the file table -- optional
int file_table_size;
// file table of the tracker -- your own design
//file_t file_table;
} ptp_tracker_t;

int tracker_sendseg(int peerconn, ptp_tracker_t *segment);
int tracker_recvseg(int peerconn, ptp_peer_t *segment);
int peer_sendseg(int trackerconn, ptp_peer_t *segment);
int peer_recvseg(int trackerconn, ptp_tracker_t *segment);

#endif

