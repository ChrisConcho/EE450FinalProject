#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <string>

#define BACKLOG 10
#define UDPPort "32484"
#define APort "30484"
#define MAXDATASIZE 100
#define localhost "127.0.0.1"
#define MAXBUFLEN 100




void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int SocketConnection(std::string protocol, const char * Port, bool Server){

	int sockfd;
	struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    int rv;

	memset(&hints, 0, sizeof hints);


	hints.ai_family = AF_UNSPEC;

	if( protocol.compare("TCP") == 0){

		hints.ai_socktype = SOCK_STREAM;

	}
	if( protocol.compare("UDP") == 0){

		hints.ai_socktype = SOCK_DGRAM;
	}
	
	hints.ai_flags = AI_PASSIVE; 
	
	const char * ipDest;
	if(Server == true){
		ipDest = NULL;
	}
	if( Server == false){
		ipDest = localhost;
	}
	rv = getaddrinfo(ipDest, Port, &hints, &servinfo);

	if (rv != 0 ) {

		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return -1;
	}

	// make a socket:

	for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if(Server == true ){
			if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof yes) == -1) {
			    perror("setsockopt");
			    exit(1);
			}

			if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1){
				close(sockfd);
				perror("server:bind");
				continue;

			}
		}
		break;
	}

	freeaddrinfo(servinfo);


	if( p == NULL){

		fprintf(stderr, "server: failed to bind \n");
		exit(1);
	}

	if(listen(sockfd,BACKLOG) == -1 && protocol.compare("TCP") == 0){

		perror("listen");
		exit(1);

	}

	return sockfd;

}


int main(void){
	

	int numbytes, udpsock, A_fd;
	char buf[MAXDATASIZE];  // listen on sock_fd, new connection on new_fd
    struct sigaction sa;
    char s[INET6_ADDRSTRLEN];
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;

	

    A_fd = SocketConnection("UDP", APort, true);

	while(1){


	    sin_size = sizeof their_addr;
	    if ((numbytes = recvfrom(A_fd, buf, MAXBUFLEN-1 , 0,
	        (struct sockaddr *)&their_addr, &sin_size)) == -1) {
	        perror("recvfrom");
	        exit(1);
	    }

	    buf[numbytes] = '\0';
	    printf("listener: packet contains \"%s\"\n", buf);


	    
	    struct addrinfo hints, *servinfo, *p;
	    int yes=1;
	    int rv;

		memset(&hints, 0, sizeof hints);


		hints.ai_family = AF_UNSPEC;
		
		hints.ai_socktype = SOCK_DGRAM;
		
		hints.ai_flags = AI_PASSIVE; 
		


		rv = getaddrinfo(localhost, UDPPort, &hints, &servinfo);

		if (rv != 0 ) {

			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
	        return -1;
		}

		// make a socket:

		for(p = servinfo; p != NULL; p = p->ai_next) {
	        if ((udpsock= socket(p->ai_family, p->ai_socktype,
	                p->ai_protocol)) == -1) {
	            perror("server: socket");
	            continue;
	        }
			break;
		}

		if( p == NULL){

			fprintf(stderr, "server: failed to bind \n");
			exit(1);
		}


	    std::string user1 = "12";
	    std::string user2 = "5";
	    std::string output = user1 + "," + user2;

	    const void * msg = output.c_str();

	    if (sendto(udpsock, msg, output.length() , 0, p->ai_addr, p->ai_addrlen) == -1){
	    	perror("send");
	    }
		
		freeaddrinfo(servinfo);



	   
	    close(udpsock);
	}
 }

