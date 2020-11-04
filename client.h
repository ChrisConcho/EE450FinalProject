#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string>
#include <iostream>
#include <arpa/inet.h>

#define PORT "33484" // the port client will be connecting to 
#define localhost "127.0.0.1"
#define MAXDATASIZE 100 // max number of bytes we can get at once 

class client{

	public:

		client(){

		}
		int sockfd, numbytes, new_fd;  
	    char buf[MAXDATASIZE];
	    struct addrinfo hints, *servinfo, *p;
	    struct sockaddr_storage their_addr; // connector's address information
	    socklen_t sin_size;

	    int rv;
	    char s[INET6_ADDRSTRLEN];

	    int TCPConnection();
};