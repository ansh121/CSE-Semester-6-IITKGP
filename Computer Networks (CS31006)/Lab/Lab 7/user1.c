/*
+++++++++++++ Anshul CHoudhary | Ayush Kumar +++++++++++++
+++++++++++++ 17CS10005 | 17CS10007 +++++++++++++
*/
#include <stdio.h> 
#include <arpa/inet.h>
#include <string.h>
 
#include "rsocket.h"

#define MAX 100 
#define ROLL 10005


char buff[MAX] ;

struct sockaddr_in addr; 
socklen_t len; 

int main(int argc,  char **argv ) { 
  
    // Creating socket file descriptor 
    int sockfd = r_socket(AF_INET, SOCK_MRP, 0);
    if(sockfd>=0){
        scanf("%s",buff);
        memset(&addr, 0, sizeof(addr)); 

        addr.sin_family = AF_INET; 
        addr.sin_port = htons(5000+2*ROLL); 
        addr.sin_addr.s_addr = INADDR_ANY; 

        int n=strlen(buff);
        for(int i = 0;i<n;i++){  
            len = sizeof(addr);
            r_sendto(sockfd, (const char *)(buff+i), 1, 0,(const struct sockaddr *) &addr, len);
        }
        r_close(sockfd); 
        return 0; 
    }
    else { 
        perror("socket creation failed"); 
        exit(1); 
    } 
    
} 