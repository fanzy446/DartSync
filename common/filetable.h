//FILE: common/peertable.h 
//
//Description: 
//
//Date: May 18, 2016


#ifndef FILETABLE_H 
#define FILETABLE_H


//each file can be represented as a node in file table
typedef struct node{
     //the size of the file
     int size;
     //the name of the file
     char *name;
     //the timestamp when the file is modified or created
     unsigned long int timestamp;
     //pointer to build the linked list
     struct node *pNext;
     //for the file table on peers, it is the ip address of the peer
     //for the file table on tracker, it records the ip of all peers which has the
     //newest edition of the file
     char *newpeerip;
}Node,*pNode;

typedef struct filetable{
	struct node *head;
};

#endif