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


#define MAX 40 
#define PORT 8181
#define SA struct sockaddr 
  
int main() 
{ 
    int sockfd, connfd,len; 
    struct sockaddr_in servaddr, cliaddr; 
   
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd < 0) { 
        printf("socket creation failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully created..\n");

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));
  
    // assign IP, PORT 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(PORT); 
  
    // connect the client socket to server socket
    int flag = connect(sockfd, (SA*)&servaddr, sizeof(servaddr));
    if ( flag != 0) { 
        printf("connection with the server failed...\n"); 
        exit(0); 
    } 
    else
        printf("connected to the server..\n");

    // Reading file name from user
    char buff[MAX],f_name[MAX];

    printf("Enter file name to read from the server : ");
    scanf("%s", f_name);
    //printf("%s\n",buff);
    write(sockfd, f_name, sizeof(f_name));

    memset(&buff, 0, sizeof(buff));
    read(sockfd, buff, sizeof(buff));

    printf("%s\n",buff);
    if(strcmp(buff,"File Found")==0){
        int file;

        if((file = open("recieved.txt", O_CREAT | O_WRONLY | O_TRUNC, 0644)) < 0) { // opening new file for writing
            printf("Client file could not be created, so closing\n");
            close(sockfd);
            return 0;
        }

        int numWords=0;
        int numBytes=0;
        int i=0,n_bytes=0;
        int wordFlag = 0; // indicates if we have encountered characters other than delimiters

        while((n_bytes=read(sockfd,buff,sizeof(buff))) > 0){
            write(file,buff,n_bytes);
            for(i=0;i<n_bytes;i++){
                if(buff[i]==',' || buff[i]==';' || buff[i]==':' || buff[i]=='.' || buff[i]==' ' || buff[i]=='\n'){
                    if(wordFlag != 0) {
                        numWords++; // if word was there before delimiter, increment
                    }
                    wordFlag = 0;
                }else{
                    wordFlag = 1; // word found
                }
            }
            numBytes+=n_bytes; // incrementing number of bytes
        }
        
        if(wordFlag == 1) { //last
            numWords++;
        }

        close(file);

        printf("File Recieved.\n Size - %d bytes , no. of words - %d\n",numBytes,numWords);
    }  
  
    // close the socket 
    close(sockfd); 
} 
