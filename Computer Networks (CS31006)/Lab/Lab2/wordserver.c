// A Simple UDP Server that sends a HELLO message
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
    struct sockaddr_in servaddr, cliaddr; 
      
    // Create socket file descriptor 
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if ( sockfd < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
      
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 
      
    servaddr.sin_family    = AF_INET; 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(8181); 
      
    // Bind the socket with the server address 
    if ( bind(sockfd, (const struct sockaddr *)&servaddr,  
            sizeof(servaddr)) < 0 ) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    
    printf("\nServer Running....\n");
  
    int n; 
    socklen_t len;
    char buffer[MAXLINE],temp[MAXLINE]; 
 
    len = sizeof(cliaddr);

    n = recvfrom(sockfd, (char *)buffer, MAXLINE, 0, 
		( struct sockaddr *) &cliaddr, &len);
	buffer[n] = '\0';

	FILE *f=fopen(buffer,"r");
	char* terminate="END";

	if(f==NULL){
		sprintf(temp,"%s","NOTFOUND ");
		sprintf(&temp[9],"%s",buffer);
		sendto(sockfd, (const char *)temp, strlen(temp), 0, 
			(const struct sockaddr *) &cliaddr, sizeof(cliaddr));
	}
	else{
		fgets(buffer,MAXLINE,f);
		sendto(sockfd, (const char *)buffer, strlen(buffer), 0, 
				(const struct sockaddr *) &cliaddr, sizeof(cliaddr));

	    while(1){
	    	n = recvfrom(sockfd, (char *)buffer, MAXLINE, 0, 
				( struct sockaddr *) &cliaddr, &len);
	    	buffer[n] = '\0';

	    	printf("Sending %s to client\n",buffer);

	    	fgets(buffer,MAXLINE,f);
	    	//printf("%s\n", buffer);
	    	sendto(sockfd, (const char *)buffer, strlen(buffer), 0, 
				(const struct sockaddr *) &cliaddr, sizeof(cliaddr));

	    	if(strcmp(buffer,terminate)==0) break;
	    }

	    fgets(buffer,MAXLINE,f);

	    fclose(f);
	}
      
    return 0; 
} 
