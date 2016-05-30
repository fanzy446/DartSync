#include "../../peer/p2p.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <arpa/inet.h>
//#include "../../common/constants.h"

void receive_data(int conn)
{
	// printf("upload_recvreqpkt: start\n");
	char buf[10260];
    char c;
    int idx = 0;
    // state can be 0,1,2,3;
    // 0 starting point
    // 1 '!' received
    // 2 '&' received, start receiving segment
    // 3 '!' received,
    // 4 '#' received, finish receiving segment
    int state = 0;
    printf("123\n");
    while(recv(conn,&c,1,0)>0) {
    	//printf("222\n");
    	printf("%c",c);
        // if (state == 0) {
        //     if(c=='!')
        //         state = 1;
        // }
        // else if(state == 1) {
        //     if(c=='&')
        //         state = 2;
        //     else
        //         state = 0;
        // }
        // else if(state == 2) {
        // 	buf[idx]=c;
        //     idx++;
        //     if(c=='!') {
        //         state = 3;
        //     }
        // }
        // else if(state == 3) {
        // 	buf[idx]=c;
        //     idx++;
        //     if(c=='#') {
        //         state = 0;
        //         idx = 0;
        //         //memcpy(pkt,buf,sizeof(p2p_request_pkg_t));
        //         // printf("upload_recvreqpkt: end\n");
        //         printf("get!!!");
        //         //return 1;
        //     }
        //     else if(c!='!') {
        //         state = 2;
        //     }
        // }
    }
    perror("upload_recvreqpkt: receive failed");
    //return -1;
}

int main(int argc , char *argv[]){
	int listenfd, connfd, n;
	struct sockaddr_in cli_addr, serv_addr;
	socklen_t clilen;

	if ((listenfd = socket (AF_INET, SOCK_STREAM, 0)) <0)
	{
		perror("Error when creating listen socket");
		return -1;
	}

	int port = atoi(argv[1]);
	printf("listening on port:%d", port);

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(port);
	bind(listenfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
	listen (listenfd, 10);
	printf("start listening!\n");

	// int running = 0;
	// upload_running_mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
	// pthread_mutex_init(upload_running_mutex, NULL);
	while(1)
	{
		// pthread_mutex_lock(upload_running_mutex);
		// if(running > MAX_UPLOAD_THREAD){
		// 	pthread_mutex_unlock(upload_running_mutex);
		// 	sleep(1);
		// 	continue;
		// }
		// running++;
		// pthread_mutex_unlock(upload_running_mutex);
		clilen = sizeof(cli_addr);
		connfd = accept(listenfd, (struct sockaddr *)&cli_addr, (socklen_t*)&clilen);
	    if (connfd < 0) {
	        perror("accept failed");
	        break;
	    }
	    printf("Connection accepted\n");

	 //    p2p_request_pkg_t* req_pkt = malloc(sizeof(p2p_request_pkg_t));
	 //    memset(req_pkt, 0, sizeof(p2p_request_pkg_t));

	    receive_data(connfd);

	    if (send(connfd, "12", 2, 0) < 0) {
	        perror("download_sendpkt: send start failed");
	        return -1;
	    }
	    	
		// printf("from ip:%s | port:%d | partition:%d\n", inet_ntoa(cli_addr.sin_addr), cli_addr.sin_port, req_pkt->partition);

		// send_thread_arg_t* send_arg = malloc(sizeof(send_thread_arg_t));
		// memset(send_arg, 0, sizeof(send_thread_arg_t));
		// send_arg->conn = connfd;
		// send_arg->req_info = req_pkt;
		// send_arg->running = &running;

		// pthread_t* upload_threads = malloc(sizeof(pthread_t));
		// pthread_create(upload_threads,NULL,upload_thd,(void*)send_arg);
	}
	//free(upload_running_mutex);
	close(listenfd);
	return -1;
}