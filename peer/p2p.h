// LICENSE ISSUES
//   ==============

//   The OpenSSL toolkit stays under a dual license, i.e. both the conditions of
//   the OpenSSL License and the original SSLeay license apply to the toolkit.
//   See below for the actual license texts.

//   OpenSSL License
//   ---------------

/* ====================================================================
 * Copyright (c) 1998-2016 The OpenSSL Project.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.openssl.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    openssl-core@openssl.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.openssl.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 */



#ifndef P2P_H 
#define P2P_H

#include "../common/constants.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <openssl/md5.h>
#include <sys/stat.h>
#include <time.h>
#include <utime.h>

typedef struct p2p_request_pkg{
	char filename[FILE_NAME_LENGTH];
	char md5key[MD5_LEN+1];
	unsigned long int timestamp;
	int partition;
} p2p_request_pkg_t;

typedef struct p2p_data_pkg{
	int size;
	char data[BLOCK_SIZE];
} p2p_data_pkg_t;

typedef struct p2p_request_arg{
	char ip[IP_LEN];
	char rootpath[FILE_NAME_LENGTH];
	char filename[FILE_NAME_LENGTH];
	char md5key[MD5_LEN+1];
	unsigned long int timestamp;
	int partition;
	int* exist;
	int* running;
} p2p_request_arg_t;

typedef struct send_thread_arg
{
	int conn;
	int* running;
	p2p_request_arg_t *req_info;
}send_thread_arg_t;

int download(char* rootpath, char* filename, int size, unsigned long int timestamp, char nodes[][IP_LEN], int numOfNodes);


void* start_listening(void* arg);

void* singleDownload(void* args);

int download_sendpkt(p2p_request_pkg_t* pkt, int conn);

int download_recvpkt(p2p_data_pkg_t* pkt, int conn);

int upload_sendpkt(p2p_data_pkg_t* pkt, int conn);

int upload_recvreqpkt(p2p_request_pkg_t* pkt, int conn);

void* upload_thd(void* arg);

int upload(int sockfd, p2p_request_pkg_t* pkg); 

char* getPath(char* rootpath, char* filename);

#endif
