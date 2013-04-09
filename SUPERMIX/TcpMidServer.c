#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <pthread.h>
#include "./UdpSend.h"

#define BUFFSIZE 625
//#define PORT 20002
#define UDP_SERVADDR "127.0.0.1"
#define UDP_SERV_PORT 20001


// This function can be modified due to the different
// behaviour this server is gonna do to response to the request msg
void get_sent_buff( char* readbuff, char* sendbuff )
{
    //strcpy(sendbuff,readbuff);
    memcpy(sendbuff,readbuff,BUFFSIZE);
}


void get_udpsent_buff(char* readbuff,char* sendbuff){
    //strcpy(sendbuff,readbuff);
    memcpy(sendbuff,readbuff,BUFFSIZE);
}


int main(int argc, char *argv[])
{
    //random variebles lose and duplicate packets
    double loserate;
    double duprate;
    double losevent;
    double dupevent;

    
    // get two random event from commendline
    if( argc != 4 ){
        printf("Usage: <packet loss rate> <packet duplicated rate> <port>\n");
        exit(1);
    }
    int PORT = atoi (argv[3]);
    loserate = atof(argv[1]);
    duprate = atof(argv[2]);
    if ( ( loserate > 1 || loserate < 0 ) || 
         ( duprate > 1) || ( duprate < 0 ) ) {
        printf("Commendline arguments error: either rate should be in range [0,1]\n");
    }

    // declare the socket ids and remoting address
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr; 

    char sendbuff[BUFFSIZE]; // buffer for sending msg 
    char readbuff[BUFFSIZE];
    int readsize = 0;
    int send_succeed = 0;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendbuff, '0', sizeof(sendbuff)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT); 

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 
    printf("Binding Succeed!\n");

    listen(listenfd, 10); 
    printf("Server is up ,listening at port : %d\n",PORT);
    randomize();
    while(1)
    {
        //accept the new request in the queue
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 
        printf("connection accept\n");
        
        int count = 0; //count the message number in one session
        // when there is request sent by client and the msg is not 'quit'
        while( (readsize = recv(connfd , readbuff , sizeof(readbuff) , 0)) > 0 )
        {
            printf("msg: %d\n", count);
            readbuff[readsize] = 0;//end the string
            if(!strcmp(readbuff,"quit\n")){
                printf("One session is closed\n");
                //close the connection with the client
                close(connfd);
                sleep(1);// wait a milisecond before accepting next request
            }
            else{
                //printf("buffer received: %s\n",readbuff);
                // send buffer by UDP to UDP echo server
                get_udpsent_buff(readbuff,sendbuff);
                losevent = get_random_rate();
                dupevent = get_random_rate(); 
                send_succeed = udp_send(sendbuff, 
                                        UDP_SERVADDR, 
                                        UDP_SERV_PORT, 
                                        readbuff,
                                        BUFFSIZE,
                                        loserate,
                                        duprate,
                                        losevent,
                                        dupevent
                                        );
                //printf("udp_send returns %s\n",sendbuff);
            }

            // prepare the send buffer
            if(send_succeed)
            {
                get_sent_buff(readbuff,sendbuff); 
                write(connfd, sendbuff, BUFFSIZE); //response to the request by sending connfd
                //printf("Response : %s\n",sendbuff);
                printf("Response Sent. %d\n",sendbuff[0]);
                count++;
            }
            else{
                printf("Fail to get response to udp server\n");
            }
        }
     }
}
