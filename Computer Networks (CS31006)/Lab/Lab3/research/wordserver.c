#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <fcntl.h> 
#include <errno.h> 

#define MAX 80 
#define PORT 8181 
#define SA struct sockaddr 
  
int main() 
{ 
    int sockfd, connfd;
    socklen_t len; 
    struct sockaddr_in servaddr, cliaddr; 
  
    // socket create and verification 
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd < 0) { 
        printf("socket creation failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully created..\n"); 
    
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr)); 

    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(PORT); 
  
    // Binding newly created socket to given IP 
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
        printf("socket bind failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully binded..\n"); 
  
    if ((listen(sockfd, 5)) != 0) { 
        printf("Listen failed...\n"); 
        exit(0); 
    } 
    else
        printf("Server listening..\n");

    len = sizeof(cliaddr); 
    connfd = accept(sockfd, (SA*)&cliaddr, &len);

    if (connfd < 0) { 
        printf("server acccept failed...\n"); 
        exit(0); 
    } 
    else
        printf("server acccepted the client...\n");

    char buff[MAX],f_name[MAX];
    
    memset(&buff, 0, sizeof(buff));
    memset(&f_name, 0, sizeof(f_name));
    read(connfd, f_name, sizeof(f_name));

    int file;
    if((file = open(f_name, O_RDONLY)) < 0) {
        sprintf(buff,"%s","File Not Found\0");
        write(connfd, buff, sizeof(buff));
        printf("File Not Found. Connection Closed ...\n");
        close(sockfd);
        return 0;
    } 

    sprintf(buff,"%s","File Found\0");
    write(connfd, buff, strlen(buff));

    int n_bytes;
    memset(&buff, 0, sizeof(buff));

    while((n_bytes = read(file,buff,MAX)) > 0){ // reading from file into buff until file finished
        write(connfd,buff,n_bytes); // writing to client from buff
    }

    close(sockfd); // closing socket
    close(file); // closing file

    printf("Requested File Sent. Connection Closed\n");  
} 
