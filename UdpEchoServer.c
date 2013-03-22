#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h> 
#include <string.h>

#define BUFLEN 5000
#define PORT 20001
#define ACK -1

void err(char *str)
{
    perror(str);
    exit(1);
}

// This function can be modified due to the different
// behaviour this server is gonna do to response to the request msg
void get_sent_buff( char* read, char* send )
{
    strcpy(send,read);
}

int main(void)
{
    struct sockaddr_in my_addr, cli_addr;
    int sockfd, i; 
    socklen_t slen=sizeof(cli_addr);
    char readbuff[BUFLEN],sendbuff[BUFLEN];
    long first_counter,second_counter;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
      err("socket");
    else 
      printf("Server : Socket() successful\n");

    bzero(&my_addr, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(PORT);
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if (bind(sockfd, (struct sockaddr* ) &my_addr, sizeof(my_addr))==-1)
      err("bind");
    else
      printf("Server : bind() successful\n");

    int counter = 0;
    //the server discards every second message
    while(1)
    {
        // listen to the port and read the message in readbuffer
        if (recvfrom(sockfd, readbuff, BUFLEN, 0, (struct sockaddr*)&cli_addr, &slen) == -1)
        {
            err("recvfrom()");
        }
        printf("Message received: %s\n",readbuff);
        //printf("Received packet from %s:%d\nData: %s\n\n",
        //       inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), readbuff);

        // if the message was duplicated, discard it 
        /*if( counter%2 == 1 )
        {
            continue;
        }*/
        // send an ACK message to the client 
        sendbuff[0] = ACK;
        printf("Before sending ACK\n");
        if (sendto(sockfd, sendbuff, BUFLEN, 0, (struct sockaddr*)&cli_addr, slen) == -1)
        {
            err("sendto()"); 
        } 
        printf("ACK sent\n");

        get_sent_buff(readbuff,sendbuff);

        // send readbuffer back to the client
        if (sendto(sockfd, sendbuff, BUFLEN, 0, (struct sockaddr*)&cli_addr, slen) == -1)
        {
            err("sendto()");  
        }
        printf("Response sent! buffer: %s\n",sendbuff);

        counter++;
    }

    close(sockfd);
    return 0;
}