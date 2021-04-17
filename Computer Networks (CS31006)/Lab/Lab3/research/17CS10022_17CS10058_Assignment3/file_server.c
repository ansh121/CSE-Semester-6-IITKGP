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

#define MAXLINE 80

int main() { 
    int sockfd, connfd;
    socklen_t len; 
    struct sockaddr_in servaddr, cliaddr; 
      
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if ( sockfd < 0 ) {  //creating socket
        perror("Socket creation failed"); 
        exit(EXIT_FAILURE); 
    } else {
        printf("Socket created successfully\n");
    }
      
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 
      
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(8181); 
      
    if(bind(sockfd,(const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 ) { // Bind the socket with the server address 
        perror("Bind failed"); 
        exit(EXIT_FAILURE); 
    } else {
        printf("Socket successfully binded..\n"); 
    }

    if(listen(sockfd,5) != 0) { // listening on port
        perror("Listen failed\n");
        exit(EXIT_FAILURE); 
    } else {
        printf("Server listening\n");
    }

    len = sizeof(cliaddr);
    connfd = accept(sockfd,(struct sockaddr *)&cliaddr, &len); // accepting connection from client
    if(connfd < 0) {
        perror("Server accept failed\n");
        exit(EXIT_FAILURE);
    } else {
        printf("Server accepting client\n");
    }

    char filename[MAXLINE];
    read(connfd,filename,sizeof(filename)); // reading filename

    int file;
    if((file = open(filename, O_RDONLY)) < 0) {
        printf("FILE NOT FOUND. Connection Closed ...\n"); // file not found
        close(sockfd);
        return 0;
    } 

    char buffer[MAXLINE];
    int readBytes;

    while((readBytes = read(file,buffer,sizeof(buffer))) > 0){ // reading from file into buffer until file finished
        write(connfd,buffer,readBytes); // writing to client from buffer
    }

    close(sockfd); // closing socket
    close(file); // closing file

    printf("File Sent. Connection closed ...\n");
}