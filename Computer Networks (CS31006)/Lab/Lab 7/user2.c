/*
+++++++++++++ Anshul CHoudhary | Ayush Kumar +++++++++++++
+++++++++++++ 17CS10005 | 17CS10007 +++++++++++++
*/
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdio.h> 
#include <string.h> 


#include "rsocket.h"


#define MAX 100 
#define ROLL 10005


int main(int argc,  char **argv ) { 
    
    char buff[MAX] ;
    struct sockaddr_in addr1, addr2; 
    socklen_t len; 

    memset(&addr1, 0, sizeof(addr1)); 
    memset(&addr2, 0, sizeof(addr2)); 

    // Creating socket file descriptor 
    int sockfd = r_socket(AF_INET, SOCK_MRP, 0);

    if(sockfd>=0){
        addr2.sin_family = AF_INET;
        addr2.sin_port = htons(5000 + 2 * ROLL);
        addr2.sin_addr.s_addr = INADDR_ANY; 

        int flag;
        flag=r_bind(sockfd, (const struct sockaddr *)&addr2, sizeof(addr2));

        if ( flag >= 0 ) { 
            len = sizeof(addr1);
            int n=MAX;
            while(n--){  
                int n = r_recvfrom(sockfd, (char *)buff, MAX, 0,  ( struct sockaddr *) &addr1, &len);
                printf("%s\n", buff); 
            }
            printf("\n");
            r_close(sockfd); 
            return 0;
        }
        else{
            perror("bind failed"); 
            r_close(sockfd);
            exit(EXIT_FAILURE); 
        }

       
    }
    else{ 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 

} 

