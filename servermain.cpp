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
#define TCPPort "33484"
#define UDPPort "32484"
#define APort "30484"
#define BPort "31484"
#define MAXDATASIZE 100
#define localhost "127.0.0.1"
#define MAXBUFLEN 100

void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}


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
	

	int sockfd, new_fd, numbytes, udpsock, A_fd, B_fd;
	char buf[MAXDATASIZE];  // listen on sock_fd, new connection on new_fd
    struct sigaction sa;
    char s[INET6_ADDRSTRLEN];
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;



	
    sockfd = SocketConnection("TCP", TCPPort, true);

    udpsock = SocketConnection("UDP", UDPPort, true);

	sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;


    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections...\n");

    while(1) {  // main accept() loop
        
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        
        if (new_fd == -1) {
            
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);

        printf("server: got connection from %s\n", s);

        if (!fork()) { // this is the child process
            close(sockfd); // child doesn't need the listener
            

            if (send(new_fd, "Hello, world!", 13, 0) == -1){
                perror("send");
            }

            if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
		        perror("recv");
		        exit(1);
		    }
		    buf[numbytes] = '\0';
		    printf("server: received '%s'\n",buf);


		    // ------------------------------------------------------------------------------------------------

		    // We have succefully set up a TCP connection with the client.

		    // Check if valid inputs

		    // decide which server to go to

		    // make UDP connection with server


		    //if Backend Server A

			struct addrinfo hints, *servinfo, *p;
		    struct sockaddr_storage their_addr; // connector's address information
		    socklen_t sin_size;
		    struct sigaction sa;
		    int yes=1;
		    int rv;

			memset(&hints, 0, sizeof hints);


			hints.ai_family = AF_UNSPEC;
			
			hints.ai_socktype = SOCK_DGRAM;
			
			hints.ai_flags = AI_PASSIVE; 
			


			rv = getaddrinfo(localhost, APort, &hints, &servinfo);

			if (rv != 0 ) {

				fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		        return -1;
			}

			// make a socket:

			for(p = servinfo; p != NULL; p = p->ai_next) {
		        if ((A_fd= socket(p->ai_family, p->ai_socktype,
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


		    if (sendto(A_fd, buf, numbytes+1 , 0, p->ai_addr, p->ai_addrlen) == -1){
                perror("sendto 1 ");
            }
			
			freeaddrinfo(servinfo);

            // buf = '';
            sin_size = sizeof their_addr;
		    if ((numbytes = recvfrom(udpsock, buf, MAXBUFLEN-1 , 0,
		        (struct sockaddr *)&their_addr, &sin_size)) == -1) {
		        perror("recvfrom");
		        exit(1);
		    }

		    close(A_fd);

		    buf[numbytes] = '\0';


		    // grab results and send to client


		    std::string output(buf);
    		std::string user1 = output.substr(0,output.find(','));
    		std::string user2 = output.substr(output.find(',')+1);
		    std::string outuput = user1 + ',' + user2;
		    const void * a = outuput.c_str();
		    if (send(new_fd, a, outuput.length() , 0) == -1){
                perror("send");
            }
            close(new_fd);
            exit(0);
        }
        close(new_fd);  // parent doesn't need this
    }

    return 0;
}
	// listen(sockfd, BACKLOG);

	// // connect!
	// addr_size = sizeof(their_addr);
	// new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);


	// connect(sockfd, res->ai_addr, res->ai_addrlen);


