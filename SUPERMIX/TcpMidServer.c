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

#define BUFFSIZE 5000
#define MAXLINE 5000
#define PORT 20002
#define UDP_SERVADDR "127.0.0.1"
#define UDP_SERV_PORT 20001


// This function can be modified due to the different
// behaviour this server is gonna do to response to the request msg
void get_sent_buff( char* read, char* send )
{
    strcpy(send,read);
}

void get_udpsent_buff(char* readbuff,char* sendbuff){
    strcpy(send,read);
}

int main(int argc, char *argv[])
{
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

    while(1)
    {
        //accept the new request in the queue
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 
        printf("connection accept");
        
        // when there is request sent by client and the msg is not 'quit'
        while( (readsize = recv(connfd , readbuff , sizeof(readbuff) , 0)) > 0 )
        {
            readbuff[readsize] = 0;//end the string
            if(!strcmp(readbuff,"quit\n")){
                printf("One session is closed\n");
                //close the connection with the client
                close(connfd);
                sleep(1);// wait a milisecond before accepting next request
            }
            else{
                printf("buffer received: %s\n",readbuff);
                // send buffer by UDP to UDP echo server
                get_udpsent_buff(readbuff,sendbuff);
                send_succeed = udp_send(sendbuff, 
                                        UDP_SERVADDR, 
                                        UDP_SERV_PORT, 
                                        readbuff,
                                        BUFFSIZE);
            }

            // prepare the send buffer
            if(send_succeed)
            {
                get_sent_buff(readbuff,sendbuff); 
                write(connfd, sendbuff, strlen(sendbuff)); //response to the request by sending connfd
            }
        }
     }
}