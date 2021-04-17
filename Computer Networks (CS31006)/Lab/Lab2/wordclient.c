// A Simple Client Implementation
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 

#define MAXLINE 1024 
  
int main() { 
    int sockfd; 
    struct sockaddr_in servaddr; 
  
    // Creating socket file descriptor 
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if ( sockfd < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
  
    memset(&servaddr, 0, sizeof(servaddr)); 
      
    // Server information 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(8181); 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
      
    int n;
    socklen_t len; 
    char buffer[MAXLINE];

    printf("Enter the name of the file to open in the server : ");
    scanf(" %s",buffer);
      
    sendto(sockfd, (const char *)buffer, strlen(buffer), 0, 
			(const struct sockaddr *) &servaddr, sizeof(servaddr));

    n = recvfrom(sockfd, (char *)buffer, MAXLINE, 0, 
			( struct sockaddr *) &servaddr, &len);
    buffer[n] = '\0';

    printf("%s",buffer);

    if(buffer[0]=='H'){

    	FILE *f=fopen("recieved.txt","w");

	    int num=1;
	    char* terminate="END";
	    len=sizeof(servaddr);

	    while(1){
	    	sprintf(buffer,"%s","WORD");
	    	sprintf(&buffer[4], "%d", num);
	    	num++;

	    	printf("Recieving %s from server\n",buffer);

			sendto(sockfd, (const char *)buffer, strlen(buffer), 0, 
				(const struct sockaddr *) &servaddr, sizeof(servaddr));

			n = recvfrom(sockfd, (char *)buffer, MAXLINE, 0, 
				( struct sockaddr *) &servaddr, &len);
	    	buffer[n] = '\0'; 

	    	if(strcmp(buffer,terminate)==0) break;
	    	fputs(buffer,f);   	
	    }
	    printf("%s\n",buffer);

	    fclose(f); 
    }


    close(sockfd);
    return 0; 
} 
