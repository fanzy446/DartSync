#include "p2p.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <arpa/inet.h>

#define MAX_DOWNLOAD_THREAD 300
#define MAX_UPLOAD_THREAD 300
#define MAX_LISTEN_NUM 300
#define LISTENING_PORT 1234


pthread_mutex_t* exist_mutex;
pthread_mutex_t* running_mutex;
pthread_mutex_t* upload_running_mutex;

int download(char* filename, int size, unsigned long int timestamp, char** nodes, int numOfNodes){
	int total = (size-1)/BLOCK_SIZE+1;
	printf("download: size %d, split into %d parts\n", size, total);
	int curNode = 0;

	int* exist = malloc(sizeof(int) * total);
	memset(exist, 0, sizeof(int) * total);
	int end = 0;
	int running = 0;
	
	exist_mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(exist_mutex,NULL);
	running_mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(running_mutex,NULL);

	//resume download
	char tmpFileName[FILE_NAME_LENGTH];
	for(int i = 0; i < total; i++){
		memset(tmpFileName, 0, FILE_NAME_LENGTH);
		sprintf(tmpFileName, "%s_%d_%d", filename, timestamp, i);
		if( access( tmpFileName, F_OK ) != -1 ) {
			exist[i] = 1;
		}
	}

	while(end != 1){
		end = 1;
		for(int i = 0; i < total; i++){
			//check if i exists on disk
			pthread_mutex_lock(exist_mutex);
			if(exist[i] != 1){
				end = 0;
			}
			if(exist[i] > 0){
				pthread_mutex_unlock(exist_mutex);
				continue;
			}
			pthread_mutex_unlock(exist_mutex);

			pthread_mutex_lock(running_mutex);
			if(running > MAX_DOWNLOAD_THREAD){
				pthread_mutex_unlock(running_mutex);
				continue;
			}
			running++;
			pthread_mutex_unlock(running_mutex);

			pthread_mutex_lock(exist_mutex);
			exist[i] = 2;
			pthread_mutex_unlock(exist_mutex);

			char* ip = nodes[curNode];
			curNode = (curNode+1)%numOfNodes;

			p2p_request_arg_t* request_args = malloc(sizeof(p2p_request_arg_t)) ;
			memset(request_args, 0, sizeof(p2p_request_arg_t));
			request_args->ip = ip;
			request_args->filename = filename;
			request_args->timestamp = timestamp;
			request_args->partition = i;
			request_args->exist = &(exist[i]);
			request_args->running = &running;
		
			pthread_t download_thread;
			pthread_create(&download_thread,NULL,singleDownload,(void*)request_args);
			
		}
		sleep(1);
	}
	free(exist_mutex);
	free(running_mutex);
	free(exist);

	//combine temporary files
	char recvFile[FILE_NAME_LENGTH];
	memset(recvFile, 0, FILE_NAME_LENGTH);
	sprintf(recvFile, "%s_recv", filename);
	FILE* recv = fopen(recvFile,"w");
	for(int i = 0; i < total; i++){
		char tmpFile[FILE_NAME_LENGTH];
		memset(tmpFile, 0, FILE_NAME_LENGTH);
		sprintf(tmpFile, "%s_%d_%d", filename, timestamp, i);

		FILE* tmp = fopen(tmpFile,"r");
		fseek(tmp,0,SEEK_END);
		int fileLen = ftell(tmp);
		fseek(tmp,0,SEEK_SET);
		char *buffer = (char*)malloc(fileLen);
		memset(buffer, 0, fileLen);
		fread(buffer,fileLen,1,tmp);
		fclose(tmp);
		remove(tmpFile);

		fwrite(buffer,fileLen,1,recv);
		free(buffer);
	}
	fclose(recv);
}

void* singleDownload(void* args){
	
	p2p_request_arg_t* request_args = (p2p_request_arg_t*) args;
	// printf("singleDownload: start %d\n", request_args->partition);

	struct sockaddr_in servaddr;
    servaddr.sin_port = htons(CONNECTION_PORT);
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(request_args->ip);

	int conn;
	int code = 0;

	while(1){
		conn = socket(AF_INET,SOCK_STREAM,0);
		if(conn<0) {
	    	perror("singleDownload: create socket failed 1.");
	    	break;
	    }
	    if(connect(conn, (struct sockaddr*)&servaddr, sizeof(servaddr))<0){
	    	perror("singleDownload: connect failed 1.");
	    	break;
	    }

	    p2p_request_pkg_t pkg;
		memset(&pkg, 0, sizeof(p2p_request_pkg_t));
		memcpy(pkg.filename, request_args->filename, strlen(request_args->filename));
		pkg.timestamp = request_args->timestamp;
		pkg.partition = request_args->partition;
		if(download_sendpkt(&pkg, conn) < 0){
			perror("singleDownload: download_sendpkt failed 1.");
			break;
		}
		int port = 0;
		//get the new port and connect to the upload thread
		printf("singleDownload: getting port...\n");
		if(recv(conn,&port,sizeof(int),0) < 0){
			perror("singleDownload: recv failed.");
			break;
		}
		close(conn);
		servaddr.sin_port = htons(port);
		printf("singleDownload: going to connect port %d\n", servaddr.sin_port);
		conn = socket(AF_INET,SOCK_STREAM,0);
		if(conn < 0) {
	    	perror("singleDownload: create socket failed 2.");
	    	break;
	    }
	    if(connect(conn, (struct sockaddr*)&servaddr, sizeof(servaddr))<0){
	    	perror("singleDownload: connect failed 2.");
	    	break;
	    }
	    p2p_data_pkg_t recv_pkg;
		memset(&recv_pkg, 0, sizeof(p2p_data_pkg_t));
	    if(download_recvpkt(&recv_pkg, conn) < 0){
			perror("singleDownload: download_recvpkt failed 2.");
	    	break;
	    }

		//store in a file
		char filename[FILE_NAME_LENGTH];
		memset(filename, 0, FILE_NAME_LENGTH);
		sprintf(filename, "%s_%d_%d", request_args->filename, request_args->timestamp, request_args->partition);
		FILE* f = fopen(filename,"w");
		fwrite(recv_pkg.data,recv_pkg.size,1,f);
		fclose(f);
		code = 1;
		break;
	}
	if(conn >= 0){
		close(conn);
	}
	pthread_mutex_lock(exist_mutex);
	(*request_args->exist) = code;
	pthread_mutex_unlock(exist_mutex);
	pthread_mutex_lock(running_mutex);
	(*request_args->running)--;
	pthread_mutex_unlock(running_mutex);
	free(request_args);
	// printf("singleDownload: end %d\n", request_args->partition);
    return code;
}

int download_sendpkt(p2p_request_pkg_t* pkt, int conn)
{
	// printf("download_sendpkt: start\n");
	char bufstart[2];
    char bufend[2];
    bufstart[0] = '!';
    bufstart[1] = '&';
    bufend[0] = '!';
    bufend[1] = '#';
    if (send(conn, bufstart, 2, 0) < 0) {
        perror("download_sendpkt: send start failed");
        return -1;
    }
    if(send(conn, pkt, sizeof(p2p_request_pkg_t),0)<0) {
        perror("download_sendpkt: send pkg failed");
        return -1;
    }
    if(send(conn, bufend,2,0)<0) {
        perror("download_sendpkt: send end failed");
        return -1;
    }
    // printf("download_sendpkt: end\n");
    return 1;
}

int download_recvpkt(p2p_data_pkg_t* pkt, int conn)
{
	// printf("download_recvpkt: start\n");
	char buf[sizeof(p2p_data_pkg_t)+2];
    char c;
    int idx = 0;
    // state can be 0,1,2,3;
    // 0 starting point
    // 1 '!' received
    // 2 '&' received, start receiving segment
    // 3 '!' received,
    // 4 '#' received, finish receiving segment
    int state = 0;
    while(recv(conn,&c,1,0)>0) {
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
        	buf[idx]=c;
            idx++;
            if(c=='!') {
                state = 3;
            }
        }
        else if(state == 3) {
        	buf[idx]=c;
            idx++;
            if(c=='#') {
                state = 0;
                idx = 0;
                memcpy(pkt,buf,sizeof(p2p_data_pkg_t));
                // printf("download_recvpkt: end\n");
                return 1;
            }
            else if(c!='!') {
                state = 2;
            }
        }
    }
    perror("download_recvpkt: receive failed");
    return -1;
}

int upload_sendpkt(p2p_data_pkg_t* pkt, int conn)
{
	// printf("upload_sendpkt: start\n");
	char bufstart[2];
    char bufend[2];
    bufstart[0] = '!';
    bufstart[1] = '&';
    bufend[0] = '!';
    bufend[1] = '#';
    if (send(conn, bufstart, 2, 0) < 0) {
        perror("upload_sendpkt: send start failed");
        return -1;
    }
    if(send(conn, pkt, sizeof(p2p_data_pkg_t),0)<0) {
        perror("upload_sendpkt: send pkg failed");
        return -1;
    }
    if(send(conn, bufend,2,0)<0) {
        perror("upload_sendpkt: send end failed");
        return -1;
    }
    // printf("upload_sendpkt: end\n");
    return 1;
}

int upload_recvreqpkt(p2p_request_pkg_t* pkt, int conn)
{
	// printf("upload_recvreqpkt: start\n");
	char buf[sizeof(p2p_request_pkg_t)+2];
    char c;
    int idx = 0;
    // state can be 0,1,2,3;
    // 0 starting point
    // 1 '!' received
    // 2 '&' received, start receiving segment
    // 3 '!' received,
    // 4 '#' received, finish receiving segment
    int state = 0;
    while(recv(conn,&c,1,0)>0) {
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
        	buf[idx]=c;
            idx++;
            if(c=='!') {
                state = 3;
            }
        }
        else if(state == 3) {
        	buf[idx]=c;
            idx++;
            if(c=='#') {
                state = 0;
                idx = 0;
                memcpy(pkt,buf,sizeof(p2p_request_pkg_t));
                // printf("upload_recvreqpkt: end\n");
                return 1;
            }
            else if(c!='!') {
                state = 2;
            }
        }
    }
    perror("upload_recvreqpkt: receive failed");
    return -1;
}

int init_listen_sock(int port){
	int listenfd, connfd, n;
	struct sockaddr_in cli_addr, serv_addr;
	socklen_t clilen;

	if ((listenfd = socket (AF_INET, SOCK_STREAM, 0)) <0)
	{
		perror("Error when creating listen socket");
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(port);
	bind(listenfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
	listen (listenfd, MAX_LISTEN_NUM);
	printf("start listening!\n");

	int running = 0;
	upload_running_mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(upload_running_mutex, NULL);
	while(1)
	{
		pthread_mutex_lock(upload_running_mutex);
		if(running > MAX_UPLOAD_THREAD){
			pthread_mutex_unlock(upload_running_mutex);
			sleep(1);
			continue;
		}
		running++;
		pthread_mutex_unlock(upload_running_mutex);
		clilen = sizeof(cli_addr);
		connfd = accept(listenfd, (struct sockaddr *)&cli_addr, (socklen_t*)&clilen);
	    if (connfd < 0) {
	        perror("accept failed");
	        break;
	    }
	    printf("Connection accepted\n");

	    p2p_request_pkg_t* req_pkt = malloc(sizeof(p2p_request_pkg_t));
	    memset(req_pkt, 0, sizeof(p2p_request_pkg_t));

	    upload_recvreqpkt(req_pkt, connfd);
	    	
		printf("from ip:%s | port:%d | partition:%d\n", inet_ntoa(cli_addr.sin_addr), cli_addr.sin_port, req_pkt->partition);

		send_thread_arg_t* send_arg = malloc(sizeof(send_thread_arg_t));
		memset(send_arg, 0, sizeof(send_thread_arg_t));
		send_arg->conn = connfd;
		send_arg->req_info = req_pkt;
		send_arg->running = &running;

		pthread_t* upload_threads = malloc(sizeof(pthread_t));
		pthread_create(upload_threads,NULL,upload_thd,(void*)send_arg);
	}
	free(upload_running_mutex);
	close(listenfd);
	return -1;
}

int upload_thd(void* arg){
	send_thread_arg_t *send_arg = (send_thread_arg_t *) arg;
	int  up_conn = -1, connfd = -1;
	struct sockaddr_in up_addr, cli_addr;

	while(1){
		if ((up_conn = socket (AF_INET, SOCK_STREAM, 0)) <0) {
			perror("upload_thd: Error when creating up_conn socket");
			break;
		}

		int nextport = 1500+rand()%15000;
		up_addr.sin_family = AF_INET;
		up_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		up_addr.sin_port = nextport;
		socklen_t len = sizeof(struct sockaddr_in);

		if(bind(up_conn, (struct sockaddr *) &up_addr, sizeof(up_addr)) < 0){
			perror("upload_thd: Error when binding socket");
			break;
		}
		listen (up_conn, 1);
		int port = ntohs(nextport);

	    if(send(send_arg->conn , &port , sizeof(int) , 0) < 0){
	        perror("upload_thd: send port failed");
	        break;
	    }
	    close(send_arg->conn);
	    send_arg->conn = -1;
	    socklen_t cli_len = sizeof(struct sockaddr_in);

		connfd = accept(up_conn, (struct sockaddr *)&cli_addr, (socklen_t*)&cli_len);
		if (connfd < 0) {
		    perror("upload_thd: accept failed");
		    break;
		}
		upload(connfd, send_arg->req_info);
		break;
	}
	pthread_mutex_lock(upload_running_mutex);
	(*send_arg->running)--;
	pthread_mutex_unlock(upload_running_mutex);
	if(send_arg->conn >= 0){
		close(send_arg->conn);
	}
	if(connfd >= 0){
		close(connfd);
	}
	if(up_conn >= 0){
		close(up_conn);
	}
    free(send_arg->req_info);
    free(send_arg);
}

int upload(int sockfd, p2p_request_pkg_t* pkg){
	p2p_data_pkg_t package;
	memset(&package, 0, sizeof(p2p_data_pkg_t));
	//if the pkg->timestamp < current timestemp

	FILE *fp;
	if((fp = fopen(pkg->filename,"rb"))!=NULL){
		fseek(fp,0,SEEK_END);
		int size;
		int total = (ftell(fp)-1)/BLOCK_SIZE+1;
		if(total-1 == pkg->partition){
			size = (ftell(fp)-1)%BLOCK_SIZE+1;
		}else{
			size = BLOCK_SIZE;
		}

		fseek(fp, pkg->partition * BLOCK_SIZE, SEEK_SET);
		fread(package.data, sizeof(char), size, fp);
		fclose(fp);
		package.size = size;
        printf("file %s #%d sended\n", pkg->filename, pkg->partition);
	}
	upload_sendpkt(&package, sockfd);
	return 1;
}