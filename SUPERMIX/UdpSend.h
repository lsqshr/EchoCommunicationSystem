#ifndef ADD_H_GUARD
#define ADD_H_GUARD
int udp_send(char* sendbuff, 
            char* UDP_SERVADDR, 
            short UDP_SERV_PORT, 
            char* readbuff,
            int BUFFSIZE);

#endif