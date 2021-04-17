/*
+++++++++++++ Anshul CHoudhary | Ayush Kumar +++++++++++++
+++++++++++++ 17CS10005 | 17CS10007 +++++++++++++
*/

#include "rsocket.h"

//GLobal VAriables for socket ---START------/
pthread_t tid;
pthread_mutex_t mlock;

int udp_fd = -1;
int recv_flags = 0;
int start_rb = 0;
int end_rb = 0;
int buffer_count = 0;
int cnt_trans = 0;
int counter = 0;

struct sockaddr_in recv_source_addr;

socklen_t recv_addr_len = 0;

/*Data Structures */
struct recv_buff_entry
{   
    struct sockaddr_in recv_addr;
    char buffer[MESSAGE_SIZE];
} * recv_buff;

typedef struct unacked_msg
{   
    struct sockaddr_in dest_addr;
    socklen_t addrlen;

    int id;
    int flags;

    time_t time;
    char msg[MESSAGE_SIZE];
    size_t msg_len;
    
} unAckTable;
unAckTable *unackd_msg_table;

struct recv_msg_id
{
    socklen_t addrlen;
    int id;
    struct sockaddr_in src_addr;
} * recv_msg_id_table;
//GLobal VAriables for socket ---END----/

// Auxilary functions not accesible by the user(hence, not in rsocket.h)
int Increment();
int HandleACKMsgReceive(int id);
int delFromUnackTable(int id);
int HandleReceive();
int HandleRetransmit();
int HandleAppMsgReceive(int id, char *inbuffer, struct sockaddr_in src_addr, socklen_t addr_len) ;
int getEmptyPlaceRecvid();
size_t combineIntString(int id, char *inbuffer, int len);
void breakIntString(int *id, char *inbuffer, int len);
unAckTable *GetEmptyPlace_unack();
// int  ADD_to_unacknowledged_message_table(unAckTable unack_msg);
// function to creat eth socket

int HandleRetransmit()
{
    time_t time_now = time(NULL);

    for (int i = 0; i < TABLE_SIZE; i++)
    {
        if (unackd_msg_table[i].id != -1){
            if(unackd_msg_table[i].time + TIMEOUT <= time_now)
            {
                ssize_t r = sendto(udp_fd, unackd_msg_table[i].msg,unackd_msg_table[i].msg_len,unackd_msg_table[i].flags,
                                   (struct sockaddr *)&unackd_msg_table[i].dest_addr,
                                   unackd_msg_table[i].addrlen);
                unackd_msg_table[i].time = time_now;

                printf("Retransmit %d\n", unackd_msg_table[i].id);
                cnt_trans++;
                if (r < 0)
                    return -1;
            }
        }
    }
    return 0;
}

int dropMessage(float p)
{
    float r = (float)rand() / ((float)RAND_MAX);
    return (r < p);
}


int r_socket(int domain, int type, int protocol)
{
    if (type == SOCK_MRP){
        if ((udp_fd = socket(domain, SOCK_DGRAM, protocol)) >= 0){
            //init tables
            recv_msg_id_table = (struct recv_msg_id *)malloc(TABLE_SIZE * sizeof(struct recv_msg_id));

            unackd_msg_table = (unAckTable *)malloc(TABLE_SIZE * sizeof(unAckTable));

            recv_buff = (struct recv_buff_entry *)malloc(BUFF_SIZE * sizeof(struct recv_buff_entry));

            for (int i = 0; i < TABLE_SIZE; i++)
            {
                recv_msg_id_table[i].id = unackd_msg_table[i].id = -1;
            }
            pthread_mutex_init(&mlock,NULL);
            start_rb =0;
            end_rb = 0;
            buffer_count = 0;
            //Thread X creation
            pthread_attr_t attr; //Set of thread attributes
            pthread_attr_init(&attr);
            int returnval = pthread_create(&tid, &attr, runnerX, NULL);

            if (returnval >= 0) return udp_fd;
            
            return -1;
        }
        else return udp_fd;
        
    }
    else return -1;

}

ssize_t sendACK(int id, struct sockaddr_in addr, socklen_t addr_len)
{
    char ACK[BUFF_SIZE];
    memset(ACK, '\0', sizeof(ACK));
    strcpy(ACK, "ACK");

    size_t returnval = combineIntString(id, ACK, -1);
    size_t ret = sendto(udp_fd, ACK, returnval, 0, (struct sockaddr *)&addr, addr_len);

    return ret;
}

int r_bind(int socket, const struct sockaddr *address, socklen_t address_len)
{
    return bind(socket, address, address_len);
}

ssize_t r_recvfrom(int sockfd, char *inbuffer, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen)
{
    if (sockfd == udp_fd){
        while (1){
            if (flags == MSG_DONTWAIT)
                break;
            else if (buffer_count > 0){
                strcpy(inbuffer, recv_buff[start_rb].buffer);
                
                if (len >= 0)
                {
                    if( len < strlen(inbuffer)) 
                        inbuffer[len] = '\0';
                }

                recv_buff[start_rb].recv_addr = recv_source_addr;
                start_rb = (start_rb + 1) % BUFF_SIZE;

                len = strlen(inbuffer);
                *src_addr = *(struct sockaddr *)&recv_source_addr;
                *addrlen = recv_addr_len;
                recv_flags = flags;

                buffer_count--;
                return len;
            }
            else
                sleep(0.001);
        }
    }
    else return -1;
}

int HandleAppMsgReceive(int id, char *inbuffer, struct sockaddr_in src_addr, socklen_t addr_len)
{
    int prsnt = 0;

    for (int i = 0; i < TABLE_SIZE; i++){
        if (recv_msg_id_table[i].id == id){
            prsnt = 1;
        }
    }

    if (prsnt==0)
    {
        strcpy(recv_buff[end_rb].buffer, inbuffer);
        recv_addr_len = addr_len;
        recv_source_addr = src_addr;

        buffer_count++;
        end_rb = (end_rb + 1) % BUFF_SIZE;
        int i = getEmptyPlaceRecvid();

        if (i >- 0){
            recv_msg_id_table[i].id = id;
            recv_msg_id_table[i].src_addr = src_addr;
            recv_msg_id_table[i].addrlen = addr_len;
        }
        else return -1;
    }

    sendACK(id, src_addr, addr_len);
    return 0;
}

int r_close(int sockfd)
{
    if (sockfd != udp_fd){
        while (1)
        {
            int flag = 0,n=TABLE_SIZE;

            for (int i = 0; i < n; i++){
                if (unackd_msg_table[i].id != -1){

                    flag = 1;
                }
            }
            if (!flag){
                break;
            }
        }
        printf("Transmission Count = %d\n", cnt_trans);
        pthread_kill(tid, SIGKILL);
        free(recv_buff);

        for (int i = 0; i < TABLE_SIZE; i++)
        {
            free(unackd_msg_table[i].msg);
        }

        free(recv_msg_id_table);
        free(unackd_msg_table);

        return close(sockfd);
    }
    else return -1;
}


void *runnerX(void *param)
{   
    struct timeval timeout;
    fd_set rfds;
    timeout.tv_sec = TIMEOUT;

    while (1)
    {
        FD_ZERO(&rfds);
        FD_SET(udp_fd, &rfds);

        int r = select(udp_fd + 1, &rfds, NULL, NULL, &timeout);
        if (r > 0)
        {
            if (FD_ISSET(udp_fd, &rfds))
            { 
                HandleReceive();
            }
        }
        else if (r<0)
        {
            perror("Select Failed\n");
        }
        else
        {
            timeout.tv_sec = TIMEOUT;
            HandleRetransmit();
        }
    }
}

int HandleACKMsgReceive(int id)
{
    printf("ACK %d\n", id);
    return delFromUnackTable(id);
}

int HandleReceive()
{

    char inbuffer[BUFF_SIZE];
    memset(inbuffer, '\0', sizeof inbuffer);

    struct sockaddr_in src_addr;

    socklen_t addr_len = sizeof(src_addr);
    int n = recvfrom(udp_fd, inbuffer, BUFF_SIZE, recv_flags, (struct sockaddr *)&src_addr, &addr_len);
    inbuffer[n] = '\0';

    if (!dropMessage(DROP_PROB)){
        int id;
        breakIntString(&id, inbuffer, n);

        if (strcmp(inbuffer, "ACK"))
            return HandleAppMsgReceive(id, inbuffer, src_addr, addr_len);
        else
            return HandleACKMsgReceive(id);
    }
    else return 0;
}

ssize_t r_sendto(int sockfd, const void *inbuffer, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen)
{

    if (sockfd == udp_fd){
        char *buff = (char *)inbuffer;
        int count = Increment();

        // ADD to unacknowledged message table
        unAckTable *unack_msg = GetEmptyPlace_unack();
        if (unack_msg != NULL){

            strcpy(unack_msg->msg, buff);
            size_t byte_final = combineIntString(count, unack_msg->msg, len);

            unack_msg->id = count;
            unack_msg->time = time(NULL);

            assert(byte_final == len + sizeof(unack_msg->id));

            unack_msg->msg_len = byte_final;
            unack_msg->flags = flags;
            unack_msg->dest_addr = *(struct sockaddr_in *)dest_addr;
            unack_msg->addrlen = addrlen;

            ssize_t ret = sendto(sockfd, unack_msg->msg,
                               unack_msg->msg_len,
                               unack_msg->flags,
                               (struct sockaddr *)&unack_msg->dest_addr,
                               unack_msg->addrlen);
            cnt_trans++;
            return ret;
        }
        else return -1;
    }
    else return -1;
}
/*-------------------------------------------------------------------------------*/
// Auxilary functions for help

int getEmptyPlaceRecvid()
{
    for (int i = 0; i < TABLE_SIZE; i++)
        if (recv_msg_id_table[i].id == -1)
            return i;
    return -1;
}

void breakIntString(int *id, char *inbuffer, int len)
{
    int *returnval;
    returnval = (int *)(inbuffer + strlen(inbuffer) + 1);
    *id = *returnval;
}

int delFromUnackTable(int id)
{
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        if (unackd_msg_table[i].id == id)
        {
            unackd_msg_table[i].id = -1;
            return 0;
        }
    }
    return -1;
}

int Increment()
{
    return ++counter;
}

size_t combineIntString(int id, char *inbuffer, int len)
{
    if (len == -1){
        len = strlen(inbuffer);
    }
    int n=len + sizeof(id);
    for (size_t i = len; i < n; i++){
        inbuffer[i] = '\0';
    }
    strcat(inbuffer + len + 1, (char *)&id);
    return len + sizeof(id);
}

unAckTable *GetEmptyPlace_unack()
{

    for (int i = 0; i < TABLE_SIZE; i++)
        if (unackd_msg_table[i].id == -1)
            return &unackd_msg_table[i];
    return NULL;
}
