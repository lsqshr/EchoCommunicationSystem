#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h> 
#include <string.h>
#include <pthread.h>
#include "./UdpSend.h"

#define SLEEPTIME 3000 // in miliseconds
#define ACK -1

void err(char *s)
{
    perror(s);
    return 0;
}


// start timer thread
void start_timer(pthread_t* timer_thread_p,
                 pthread_t* recv_thread_p,
                 pthread_attr_t attr)
{
    void* timer_sleep(void* recv_thread_p)
    {
        int interval = SLEEPTIME/1000;
        for(int i = 0 ; i<interval ; i++){
            sleep(1);
            printf("timer count 1 second\n");
        }

        //if the recv_thread is running, kill it   
        pthread_cancel(*((pthread_t*)recv_thread_p)); 
    }

    pthread_create(timer_thread_p,&attr,timer_sleep,(void*)recv_thread_p);
}

struct pattr{
    pthread_t *timer_thr_p;
    char* readbuff;
    int* acked_p;
    int sock;
    struct sockaddr_in serv_addr;
    int BUFFSIZE;
};

// start thread to try to receive ACK msg from server
struct pattr* wait_for_server(pthread_t* recv_thread_p,
                     pthread_t* timer_thread_p,
                     pthread_attr_t attr,
                     char* readbuff,
                     int BUFFSIZE,
                     int* acked_p,
                     int sock,
                     struct sockaddr_in serv_addr)
{

    struct pattr* attrs = (struct pattr*) malloc(sizeof(struct pattr));

    attrs->timer_thr_p = timer_thread_p;
    attrs->readbuff = readbuff;
    attrs->acked_p = acked_p;
    attrs->sock = sock;
    attrs->serv_addr = serv_addr;
    attrs->BUFFSIZE = BUFFSIZE;

    void* recv_msg(void* pattr_t){ 
        struct pattr* attr = (struct pattr*)pattr_t;
        socklen_t addrlen = sizeof(attr->serv_addr);
        if (recvfrom(attr->sock, attr->readbuff, attr->BUFFSIZE, 0, (struct sockaddr*)&(attr->serv_addr),&addrlen )==-1)
            err("recvfrom()"); 
        if( attr->readbuff != NULL && attr->readbuff[0] == ACK ){
            // received ACK from the UDP server  
            *(attr->acked_p) = 1;
            // kill the timer_thr
            pthread_cancel(*(attr->timer_thr_p));
        }
    }

    //start the recv_thread 
    pthread_create(recv_thread_p,&attr,recv_msg,(void*)attrs);

    return attrs;
}

void randomize(){
    srand(time(NULL));
}

double get_random_rate()
{
    return ((double) (rand()%100000) )  / (100000.0);
}

int udp_send(char* sendbuff, 
             char* UDP_SERVADDR, 
             short UDP_SERV_PORT, 
             char* readbuff,
             int BUFFSIZE,
             double loserate,
             double duprate,
             double losevent,
             double dupevent
             )
{
    struct sockaddr_in serv_addr;
    int sockfd, i, slen=sizeof(serv_addr);
    void* timer_status;
    void* receive_status;
    pthread_attr_t attr;
    pthread_t timer_thread,recv_thread;
    struct pattr *recv_thread_attr;// to free memory of recv thread 
    int acked = 0; // flag for if an ack msg has been received

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
        err("socket");

    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(UDP_SERV_PORT);

    if (inet_aton(UDP_SERVADDR, &serv_addr.sin_addr)==0)
    {
        fprintf(stderr, "inet_aton() failed\n");
        return 0;
    }

    printf("Client started.\n");

    if(strcmp(sendbuff,"quit") == 0)
      return 0;

    int first = 1;
    // keep trying to send sendbuff to the server, until receive ACK
    do{
        if(!first){
           printf("time out... try send data again\n");
            losevent = get_random_rate();
            dupevent = get_random_rate();
        }
        first = 0;

        // send the sendbuff to server according to the loserate
        printf("losevent:%f dupevent:%f\nloserate:%f duperate:%f\n",losevent,dupevent,loserate,duprate);
        if( losevent > loserate ) 
        {
            if (sendto(sockfd, sendbuff, BUFFSIZE, 0, (struct sockaddr*)&serv_addr, slen)==-1)
                err("sendto()");

        }
        else{
            printf("Package lost!\n");
        }

        
        if( dupevent < duprate ){
            if (sendto(sockfd, sendbuff, BUFFSIZE, 0, (struct sockaddr*)&serv_addr, slen)==-1)
                err("sendto()");
            printf("Package Duplicatly Sent!\n");
        }


        // The following one of the two threads will kill the other one, when it get through
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

        // start a timer in another thread, to count 3 seconds,
        start_timer(&timer_thread,&recv_thread,attr);

        // start a thread to wait for the server's msg
        // try to receive the ack msg
        recv_thread_attr = wait_for_server(&recv_thread,
                                           &timer_thread,
                                           attr,
                                           readbuff,
                                           BUFFSIZE,
                                           &acked,
                                           sockfd,
                                           serv_addr);

        //wait till two threads both finish
        pthread_attr_destroy(&attr);
        pthread_join(timer_thread,&timer_status);
        pthread_join(recv_thread,&receive_status);
        free(recv_thread_attr);

    }while(!acked);//while we receive the ACK msg from server

    // receive the real data
    if (recvfrom(sockfd, readbuff, BUFFSIZE, 0, (struct sockaddr*)&serv_addr,&slen)==-1)
        err("recvfrom()"); 

    close(sockfd);
    return 1;
}

