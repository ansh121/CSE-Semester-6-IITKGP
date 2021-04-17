#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include<fcntl.h> 
#include<errno.h> 
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <signal.h>

#define MAX 65535
#define PORT 8181
#define SA struct sockaddr 

int main(int argc, char* argv[]){

    signal(SIGPIPE, SIG_IGN);

    if(argc < 4){
        printf("Proxy Server Info Not entered correctly");
    }

    // char instiProxy[100] = "172.16.2.30";
    // char instiPort[4] = "8080";
    int sockfd, connfd, len, pcToinsti; 
    int client_conn[100], client_out[100];
    int num_client = 100;
    memset(client_conn, 0, sizeof(client_conn));
    memset(client_out, 0 , sizeof(client_out));
    struct sockaddr_in servaddr, cli, insti; 

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    fd_set r_set, w_set; 
    pcToinsti = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == -1) { 
        printf("Socket creation failed...\n"); 
        exit(0); 
    } 
    else printf("Socket successfully created..\n"); 

    memset(&servaddr,0, sizeof(servaddr)); 
    memset(&insti, 0, sizeof(insti));
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    servaddr.sin_port = htons(atoi(argv[1]));

    insti.sin_family = AF_INET;
    insti.sin_addr.s_addr = inet_addr(argv[2]);
    insti.sin_port = htons(atoi(argv[3]));

    if((connect(pcToinsti, (SA*)&insti, sizeof(insti)))!= 0){
        printf("Connection to Insti Proxy Server Failed\n");
        exit(0);
    }
    else{
        printf("Connected to Insti Proxy\n");
    }
    int status = fcntl(sockfd, F_SETFL, O_NONBLOCK);
    status = fcntl(pcToinsti, F_SETFL, O_NONBLOCK);

    int enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    error("setsockopt(SO_REUSEADDR) failed");

    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
        printf("Socket bind failed...\n"); 
        exit(0); 
    } 

    if(listen(sockfd, 100) < 0){
    printf("Error in Listen\n");
    exit(0);
    }

    len = sizeof(cli); 
    printf("Waiting For Connection..\n");
    FD_ZERO(&r_set);
    FD_ZERO(&w_set);
    char buff[MAX];
    int i , curr_sd, curr_rd, max_sd = 0, totalConnection = 0;
    struct timeval timeOut;
    timeOut.tv_sec = 1;
    timeOut.tv_usec = 0;
    int tem = 0;
    while(1){
        if(totalConnection< 100){
            // printf("\r Listening: %d", tem++);
            FD_ZERO(&r_set);
            FD_ZERO(&w_set);
            FD_SET(0, &r_set);
            max_sd = sockfd;
            FD_SET(sockfd, &r_set);
            for(i = 0;i< num_client;i++){
                curr_sd = client_conn[i];
                curr_rd = client_out[i];
                if(curr_sd > 0){
                    FD_SET(curr_sd, &r_set);
                    FD_SET(curr_rd, &w_set);
                    FD_SET(curr_sd, &w_set);
                    FD_SET(curr_rd, &r_set);
                }
                if(curr_sd > max_sd){
                    max_sd = curr_sd;
                }
                max_sd = (max_sd < curr_rd)?curr_rd:max_sd;
            }
            // printf("Max SD: %d\n", max_sd);
            int activity = select(max_sd+1, &r_set, &w_set, NULL,&timeOut);
            // printf("\r Waiting at Select Total Connections: %d", totalConnection);
            int tempSoc;
            if(activity <= 0)continue;
            if(FD_ISSET(0, &r_set)){
                memset(buff, 0, MAX);
                int a = read(0, buff, sizeof(buff));
                buff[a] = '\0';
                printf("Enter: %s\n ", buff);
                // printf(".\n");
                if(strcmp(buff, "exit\n") == 0){
                    printf("Exit Command Recieved\n");
                    for(i = 0;i< num_client;i++ ){
                        if( client_conn[i]  != 0){
                            close(client_conn[i]);
                            close(client_out[i]);
                        }
                    }
                    close(sockfd);
                    close(pcToinsti);
                    return 0;
                }
            }
            if(FD_ISSET(sockfd, &r_set)){
                if((tempSoc = accept(sockfd, (SA*)&cli, &len)) > 0 ){
                    char clientDet[100];
                    inet_ntop(AF_INET, &(cli.sin_addr), clientDet, 100);
                    int clientPort = (int)ntohs(cli.sin_port);
                    printf("Connection Recieved from %s Port:%d\n", clientDet, clientPort);
                    totalConnection++;
                    printf("Total Connection %d\n", totalConnection);
                }
                else{
                    printf("ERROR IN ACCEPT\n");
                    // exit(0);
                }
                for(i = 0;i<num_client;i++){
                    if(client_conn[i] == 0 ){
                        client_conn[i] = tempSoc;
                        if((client_out[i] = socket(AF_INET, SOCK_STREAM, 0) )== -1){
                            printf("Socket Client_Out Err\n");
                        }
                        if((connect(client_out[i], (SA*)&insti, sizeof(insti)))!= 0){
                            printf("Connection to Insti Proxy Server Failed From Inside while loop\n");
                            exit(0);
                        }
                        if (fcntl(client_out[i], F_SETFL, O_NONBLOCK) == -1) {
                            printf("fcntl() failed: %s\n", strerror(errno));
                            exit(EXIT_FAILURE);
                        }
                        break;
                    }
                }			
            }
            // printf("ISSET Connection Accepted Done\n");
            for(i=0;i< num_client;i++){
                if(FD_ISSET(client_conn[i], &r_set) && FD_ISSET(client_out[i], &w_set)){
                    int a = read(client_conn[i], buff, sizeof(buff));
                    int b = send(client_out[i], buff, a, 0);
                    if( b == -1){
                        switch(errno){
                            case EPIPE:
                                close(client_conn[i]);
                                client_conn[i] = 0;
                                close(client_out[i]);
                                client_out[i] = 0;
                                printf("Connection Closed \n");
                                totalConnection--;
                        }
                    }
                    if(errno == EPIPE)continue;
                }
                if(FD_ISSET(client_out[i], &r_set) && FD_ISSET(client_conn[i], &w_set)){
                    int a = read(client_out[i], buff, sizeof(buff));
                    int b = send(client_conn[i], buff, a, 0);
                    if( b == -1){
                        switch(errno){
                            case EPIPE:
                                close(client_conn[i]);
                                client_conn[i] = 0;
                                close(client_out[i]);
                                client_out[i] = 0;
                                printf("Connection Closed \n");
                                totalConnection--;
                        }
                    }
                    if(errno == EPIPE)continue;
                }		
            }
        // printf("ISSET Done Throught Client\n");
        }
    }
    // printf("\nExiting....\n");  
}