#include <arpa/inet.h> 
#include <errno.h> 
#include <netinet/in.h> 
#include <signal.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <netdb.h>
#include <stdbool.h>
#include <fcntl.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/wait.h>
#include <signal.h>

#define MAXLEN 65535
#define MAXREQ 100
#define STDIN 0

bool check_flag(int flag, char* error){
    if(flag<0){
        printf("%s \n", error);
        return false;
    }
    else{
        return true;
    }
}

int max_socket(int x1,int x2,int x3){
	int m = x1>x2?x1:x2;
	return x3>m?x3:m;
}

int main(int argc, char const *argv[]){

	if(argc!=4){
		printf("Parameters not enter correctly\n Usage- ./a.out client_port kgp_proxy kgp_port \n");
        return 0;
	}

    printf("Proxy running on port %s. Forwarding all connections to %s:%s \n", argv[1],argv[2],argv[3]);

    int sockfdi,sockfdo;
    int client_socket[MAXREQ], out_socket[MAXREQ];
    for (int i = 0; i < MAXREQ; ++i)
    {
        client_socket[i]=0;
        out_socket[i]=0;
    }
    unsigned char buffer[MAXLEN];
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr, kgp_proxy;
    // overwriting servaddr and cliaddr memory blocks with 0 for initilisation
    memset(&servaddr,0,sizeof(servaddr));
    memset(&cliaddr,0,sizeof(cliaddr));

    // creating a socket for the client
    sockfdi = socket(AF_INET, SOCK_STREAM, 0);    
    if(!check_flag(sockfdi,"socket creation failed")){
        return 0;
    }
    // making client socket not blocking
    int flag = fcntl(sockfdi, F_SETFL, O_NONBLOCK);
    if(!check_flag(flag,"Could not make port non-blocking")){
        return 0;
    }
    // creating a socket for the kgp-proxy server
    sockfdo = socket(AF_INET, SOCK_STREAM, 0);
    if(!check_flag(sockfdo,"socket creation failed")){
        return 0;
    }
    // defining some attr for kgp_proxy
    kgp_proxy.sin_family = AF_INET;
    kgp_proxy.sin_addr.s_addr = inet_addr(argv[2]);
    kgp_proxy.sin_port = htons(atoi(argv[3]));

    flag=connect(sockfdo, (struct sockaddr *)&kgp_proxy, sizeof(kgp_proxy));
    if(!check_flag(flag,"connection to kgp proxy failed")){
        return 0;
    }    
    flag = fcntl(sockfdo, F_SETFL, O_NONBLOCK);
    if(!check_flag(flag,"Could not make port non-blocking")){
        return 0;
    }

    // server parameters
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(atoi(argv[1])); 
    // enable reusability of ports
    int enable = 1;
    flag = setsockopt(sockfdi, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
    if(!check_flag(flag,"setting sockets to be reusable failed")){
        return 0;
    }
    // bind the server
    flag = bind(sockfdi, (struct sockaddr*)&servaddr, sizeof(servaddr));
    if(!check_flag(flag,"Bind failed")){
        return 0;
    }
    // Listen for incoming connection
    listen(sockfdi,MAXREQ);

    // create 2 sets, one fore reading, one for writing
    fd_set readfds,sendfds;
    FD_ZERO(&readfds);
    FD_ZERO(&sendfds);
    // tracks number of active connections
    int active_conn=0;
    // define the signal
    signal(SIGPIPE, SIG_IGN);
    // Main loop
    while(1)
    {	
    	// maintain only 100 active connections at a time
        if(active_conn<MAXREQ){
        	// initiliaze set
            FD_ZERO(&readfds);
            FD_ZERO(&sendfds);

            // add sockfdi and STDIN in read set
            FD_SET(sockfdi, &readfds);
            //FD_SET(sockfdo, &sendfds)
            FD_SET(STDIN, &readfds);
            // track the max return of socket
            int max_num=sockfdi;

            // adding active connections back to read set
            for (int i = 0; i < MAXREQ; ++i)
            {
                if(client_socket[i]>0){
                    FD_SET(client_socket[i], &readfds);
                    FD_SET(client_socket[i], &sendfds);
                    FD_SET(out_socket[i], &readfds);
                    FD_SET(out_socket[i], &sendfds);
                }
                max_num=max_socket(max_num,client_socket[i],out_socket[i]);
            }

            int curr = select(max_num+1, &readfds,&sendfds,NULL,NULL);
            //printf("selected\n");

            if(curr<=0)continue;
            // if STDIN is in subset of readfd
            if(FD_ISSET(STDIN, &readfds)){
            	//printf("here\n");
            	memset(buffer,0,sizeof(buffer));
            	// read from STDIN
            	int a =read(STDIN,buffer, sizeof(buffer));
            	buffer[a]='\0';
                if(strncmp(buffer, "exit", 4 ) == 0){
                    printf("Exit Command Recieved\n");
                    for(int i = 0;i< MAXREQ;i++ ){
                    	// close active connections
                        if( client_socket[i]  != 0){
                            close(client_socket[i]);
                            close(out_socket[i]);
                        }
                    }
                    // close master sockets
                    close(sockfdi);
                    close(sockfdo);
                    return 0;
                }
            }
            // if client socket is member of read
            if(FD_ISSET(sockfdi, &readfds)){
                clilen = sizeof(cliaddr);
                int new_socket= accept(sockfdi, (struct sockaddr *)&cliaddr, &clilen);
                active_conn++;
                if(!check_flag(new_socket,"accept failed")){
                	continue;
                }
                printf("connection accepted from: %s:%d \n",inet_ntoa(cliaddr.sin_addr),ntohs(cliaddr.sin_port));
                //printf("PRINTED HERE\n");
                // add created connection to the list
                for (int i = 0; i < MAXREQ; ++i)
                {
                    if(client_socket[i]==0){
                        client_socket[i] = new_socket;
                        out_socket[i] = socket(AF_INET, SOCK_STREAM, 0);
                        if(!check_flag(out_socket[i],"socket creation failed")){
                        	return 0;
                        }
                        flag = connect(out_socket[i], (struct sockaddr *)&kgp_proxy,sizeof(kgp_proxy));
                        if(!check_flag(flag,"connection to kgp proxy failed")){
                        	return 0;
                        }
                        flag = fcntl(out_socket[i], F_SETFL, O_NONBLOCK);
                        if(!check_flag(flag, "making ports non blocking failed")){
                        	return 0;
                        }
                        break;
                    }
                }
            }
            // check open connections and transfer data
            for (int i = 0; i < MAXREQ; ++i)
            {
            	// if client sends something to server
                if(FD_ISSET(client_socket[i], &readfds) && FD_ISSET(out_socket[i], &sendfds)){
                	// read from client socket
                    int a=  read(client_socket[i], buffer, sizeof(buffer));
                    buffer[a]= '\0';
                    // send to kgp-proxy
                    flag = send(out_socket[i], buffer, a, 0 );
                    // if connection has been cosed from server
                    if(flag<0){
                            switch(errno){
                            case EPIPE:
                            	printf("closing connection with sockfd= %d \n",client_socket[i]);
                                close(client_socket[i]);
                                client_socket[i] = 0;
                                close(out_socket[i]);
                                out_socket[i] = 0;
                                active_conn--;
                        }
                    }
                    if(errno == EPIPE) continue;
                }
                // when kgp-proxy sends something to user
                if(FD_ISSET(out_socket[i], &readfds) && FD_ISSET(client_socket[i], &sendfds)){
                	// read from kgp-proxy socket
                    int a = read(out_socket[i], buffer, sizeof(buffer));
                    buffer[a]= '\0';
                    // send to client
                    flag = send(client_socket[i], buffer,a, 0 );
                    // if client has already closed connection
                    if(flag<0){
                            switch(errno){
                            case EPIPE:
                            	printf("closing connection with sockfd= %d \n",client_socket[i]);
                                close(client_socket[i]);
                                client_socket[i] = 0;
                                close(out_socket[i]);
                                out_socket[i] = 0;
                                active_conn--;
                        }
                            // continue;
                        }
                    if(errno == EPIPE) continue;
                }
            }

        }
    }
}