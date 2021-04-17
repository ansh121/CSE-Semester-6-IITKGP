/********************
Name - Anshul Choudhary | Ayush Kumar
Roll No - 17CS10005 | 17CS10007
Assignment No - 5 (Proxy Server)
********************/

#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <fcntl.h> 
#include <errno.h> 
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <signal.h>

#define MAX 65535
#define PORT 8181
#define SA struct sockaddr 

int out_flag_single(int flag, char str[100]){
    if(flag==-1){
        printf("%s",str);
        return 1;
    }
    return 0;
}

int main(int arg_count, char* argv[]){

    signal(SIGPIPE, SIG_IGN);

    if(arg_count < 4) printf("Incorrect Input\n");

    int sockfd , sockPcInsti; 
    int connfd, len;
    struct sockaddr_in sim_addr, cli, insti_addr;
    int incoming_conn[100], forward_conn[100], max_conn_count = 100;

    int listenport, instiport;
    listenport=atoi(argv[1]);
    instiport=atoi(argv[3]);

    memset(incoming_conn, 0, sizeof(incoming_conn));
    memset(forward_conn, 0 , sizeof(forward_conn)); 

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    sockPcInsti = socket(AF_INET, SOCK_STREAM, 0); 

    if (sockfd == -1) { 
        printf("Socket creation failed...\n"); 
        exit(0); 
    } 
    printf("Socket successfully created..\n"); 

    memset(&sim_addr,0, sizeof(sim_addr)); 
    memset(&insti_addr, 0, sizeof(insti_addr));

    sim_addr.sin_family = AF_INET; 
    insti_addr.sin_family = AF_INET;

    sim_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    insti_addr.sin_addr.s_addr = inet_addr(argv[2]);

    sim_addr.sin_port = htons(listenport);
    insti_addr.sin_port = htons(instiport);

    int flag;

    flag=connect(sockPcInsti, (SA*)&insti_addr, sizeof(insti_addr));
    if(flag!= 0){
        printf("Connection to %s:%s Failed\n",argv[2],argv[3]);
        exit(0);
    }
    printf("Connected to %s:%s\n",argv[2],argv[3]);

    int status = fcntl(sockfd, F_SETFL, O_NONBLOCK);
    status = fcntl(sockPcInsti, F_SETFL, O_NONBLOCK);

    int enable = 1;
    flag=setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
    if (flag < 0) perror("setsockopt failed");
    
    flag=bind(sockfd, (SA*)&sim_addr, sizeof(sim_addr));
    if (flag != 0) { 
        printf("Socket bind failed...\n"); 
        exit(0); 
    } 

    flag=listen(sockfd, 100);
    if(flag < 0){
        printf("Listen Error\n");
        exit(0);
    }

    len = sizeof(cli); 
    printf("Proxy running on port %s. Forwarding all Connections to %s:%s ..\n", argv[1],argv[2],argv[3]);

    fd_set r_set, w_set;
    FD_ZERO(&r_set); FD_ZERO(&w_set);
    char buff[MAX];
    int i , curr_sd, curr_rd, max_sd = 0;
    struct timeval time_out;
    time_out.tv_sec = 1;
    time_out.tv_usec = 0;
    int tem = 0;
    while(1){
            FD_ZERO(&r_set); FD_ZERO(&w_set); FD_SET(0, &r_set);
            max_sd = sockfd;
            FD_SET(sockfd, &r_set);

            i=0;
            while(i< max_conn_count){
                curr_rd = forward_conn[i];

                if(incoming_conn[i] > 0){
                    FD_SET(incoming_conn[i], &r_set);
                    FD_SET(curr_rd, &w_set);
                    FD_SET(incoming_conn[i], &w_set);
                    FD_SET(curr_rd, &r_set);
                }

                max_sd = (incoming_conn[i] > max_sd)?incoming_conn[i]:max_sd;
                max_sd = (max_sd > curr_rd)?max_sd:curr_rd;
                i++;
            }

            int actv = select(max_sd+1, &r_set, &w_set, NULL,&time_out);

            int tempSoc;
            if(actv <= 0) continue;

            flag=FD_ISSET(0, &r_set);
            if(flag){
                memset(buff, 0, MAX);
                int flag = read(0, buff, sizeof(buff));
                buff[flag] = '\0';
                printf("Enter: %s\n ", buff);

                int cmp=strcmp(buff, "exit\n");
                if(cmp == 0){
                    printf("Exit Command Recieved\n");

                    i = 0;
                    while(i< max_conn_count){
                        if( incoming_conn[i]  != 0){
                            close(incoming_conn[i]);
                            close(forward_conn[i]);
                        }
                        i++;
                    }
                    close(sockfd);
                    close(sockPcInsti);
                    return 0;
                }
            }
            if(FD_ISSET(sockfd, &r_set)){
                if((tempSoc = accept(sockfd, (SA*)&cli, &len)) > 0 ){
                    char clientDet[100];
                    inet_ntop(AF_INET, &(cli.sin_addr), clientDet, 100);
                    int clientPort = (int)ntohs(cli.sin_port);
                    printf("Connection Recieved from %s Port:%d\n", clientDet, clientPort);
                }
                else{
                    printf("ERROR IN ACCEPT\n");
                    // exit(0);
                }
                i = 0;
                while(i<max_conn_count){
                    if(!(incoming_conn[i]) ){
                        incoming_conn[i] = tempSoc;

                        forward_conn[i] = socket(AF_INET, SOCK_STREAM, 0);
                        flag=forward_conn[i];
                        out_flag_single(flag,"Socket Client_Out Err\n");
                        
                        flag=connect(forward_conn[i], (SA*)&insti_addr, sizeof(insti_addr));
                        if(out_flag_single(flag,"Connection to Insti Proxy Server Failed From Inside while loop\n")){
                            exit(0);
                        }

                        flag=fcntl(forward_conn[i], F_SETFL, O_NONBLOCK);
                        if (flag == -1) {
                            printf("fcntl() failed: %s\n", strerror(errno));
                            exit(EXIT_FAILURE);
                        }
                        break;
                    }
                    i++;
                }			
            }
            i=0;
            while(i< max_conn_count){
                flag=FD_ISSET(incoming_conn[i], &r_set);
                int flag2=FD_ISSET(forward_conn[i], &w_set);

                if( flag && flag2){
                    flag = read(incoming_conn[i], buff, sizeof(buff));
                    flag2 = send(forward_conn[i], buff, flag, 0);

                    if( flag2 == -1){
                        if(errno == EPIPE){
                                close(incoming_conn[i]);
                                close(forward_conn[i]);
                                incoming_conn[i] = forward_conn[i] = 0;
                                printf("Connection Closed \n");
                            }
                    }
                    if(errno == EPIPE) continue;
                }
                if(FD_ISSET(forward_conn[i], &r_set) && FD_ISSET(incoming_conn[i], &w_set)){
                    flag = read(forward_conn[i], buff, sizeof(buff));
                    flag2 = send(incoming_conn[i], buff, flag, 0);
                    if( flag2 == -1){
                        switch(errno){
                            case EPIPE:
                                close(incoming_conn[i]);
                                incoming_conn[i] = 0;
                                close(forward_conn[i]);
                                forward_conn[i] = 0;
                                printf("Connection Closed \n");
                        }
                    }
                    if(errno == EPIPE) continue;
                }
                i++;		
            }
    }
}