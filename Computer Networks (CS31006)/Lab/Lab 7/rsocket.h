#ifndef RSOCKET_H
#define RSOCKET_H
/*
+++++++++++++ Anshul CHoudhary | Ayush Kumar +++++++++++++
+++++++++++++ 17CS10005 | 17CS10007 +++++++++++++
*/
#include <stdio.h>
#include <string.h> 
#include <stdlib.h>
#include <time.h>  
#include <unistd.h>  
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h> 
#include <pthread.h> 
#include <signal.h>
#include <assert.h>
// To remove redundant headers

#define MESSAGE_SIZE 100
#define DROP_PROB 0.2
#define SOCK_MRP 153
#define TABLE_SIZE 100
#define TIMEOUT 2
#define BUFF_SIZE 100

int dropMessage(float p);

ssize_t r_sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);

ssize_t r_recvfrom(int sockfd, char *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);

int r_socket(int domain, int type, int protocol);

int r_bind(int socket, const struct sockaddr *address, socklen_t address_len);

int r_close(int fd);

void *runnerX(void* param);
#endif


//user1.c



//usr2.c



//rsocket.c

