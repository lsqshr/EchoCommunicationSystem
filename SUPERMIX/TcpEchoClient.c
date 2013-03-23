#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 

#define BUFFSIZE 1023
#define SERVERPORT 20001
#define LOCALHOST "127.0.0.1"


int main(int argc, char *argv[])
{
    int sockfd = 0, readsize = 0;
    char recvbuff[BUFFSIZE], sendbuff[BUFFSIZE];
    struct sockaddr_in serv_addr; 

    memset(recvbuff, '0',sizeof(recvbuff));
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    } 

    memset(&serv_addr, '0', sizeof(serv_addr)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(20001);

    if(inet_aton(LOCALHOST, &serv_addr.sin_addr)<=0)
    {
        printf("\n inet_aton error occured\n");
        return 1;
    } 

    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
       printf("\n Error : Connect Failed \n");
       return 1;
    } 


    while(1)
    {   
        //get string to send from terminal
        printf("New Message To Echo: ");
        fgets(sendbuff,BUFFSIZE,stdin);
        if(strlen(sendbuff) == 0 ){
            printf("Sorry, you are trying to send an empty message\n");
            break;  
        }

        //send the message to server
        if( send(sockfd , sendbuff , strlen(sendbuff) , 0) < 0)
        {
            puts("Send failed");
            return 1;
        }

        readsize = recv(sockfd, recvbuff, sizeof(recvbuff)-1,0);
        if( readsize > 0 )
        {
            recvbuff[readsize] = 0;
            printf("%s\n",recvbuff);
        }
        else{
            break;
        }

    }

    if( readsize < 0)
    {
        printf("\n Server closed connection \n");
    } 

    return 0;
}