#ifndef CONSTANTS_H
#define CONSTANTS_H

#define BLOCK_SIZE 32768

#define FILE_NAME_LENGTH 128

#define CONNECTION_PORT 1234

#define WEB_PORT 1237

#define TEST 2

#define TRACKERPORT 5008

#define IP_LEN 45

#define HEARTRATE 60

#define MAX_PEERS 16

#define LEN_FILE_NAME 128

#define MAX_FILES 100

#define FILE_MONITOR_INTERVAL 1000000000 // IN NANOSEC (10^-9)

#define TEMPORARY_POSTFIX "dartsync"


// MUST REDEFINE THE FOLLOWING- unknown what to set them as right now
#define PROTOCOL_LEN 0
#define RESERVED_LEN 8

#define MD5_LEN 32

#define FLAG_SAME "__FLAG_SAME__"

#endif
