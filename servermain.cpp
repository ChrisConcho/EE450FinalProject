#include <iostream>
#include "servermain.h"
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

void serverMain::UDPConnections(int backendServer){

    socklen_t sin_size;
    struct addrinfo *p;
    struct sigaction sa;
    int yes=1;
    int rv;

	memset(&hints, 0, sizeof hints);


	hints.ai_family = AF_UNSPEC;
	
	hints.ai_socktype = SOCK_DGRAM;
	
	hints.ai_flags = AI_PASSIVE; 
	
	if ( backendServer == 1){

		rv = getaddrinfo(localhost, APort, &hints, &servinfoA);
	}
	if( backendServer == 2){

		rv = getaddrinfo(localhost, BPort, &hints, &servinfoB);
	}

	if (rv != 0 ) {

		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
       
	}

	if(backendServer == 1){

		for(pA = servinfoA; pA != NULL; pA = pA->ai_next) {
			
        	if ((Asock = socket(pA->ai_family, pA->ai_socktype,
                        pA->ai_protocol)) == -1) {
                    perror("server: socket");
                    continue;
                }
     		break;
        }

        if( pA == NULL){

			fprintf(stderr, "server: failed to bind \n");
		}
    }
    if(backendServer == 2)
    {
    	for(pB = servinfoB; pB != NULL; pB = pB->ai_next){
	    	
	    	if ((Bsock = socket(pB->ai_family, pB->ai_socktype,
	                    pB->ai_protocol)) == -1) {
	                perror("server: socket");
	                continue;
	    	}
			break;
		}

		if( pB == NULL){

			fprintf(stderr, "server: failed to bind \n");
		}
	}

}

void serverMain::SendAndRcv(int send, int backendServer, char buf[]){
	
	int numbytes;
	socklen_t sin_size;
	struct sockaddr_storage their_addr;

	if ( SocketConnection("UDP", UDPPort, true) != 0) {
    	std::cout<< "An Error has occured in the allocating a UDP socket"<<std::endl;
    	exit(1);
    }


	if (send == 1){

		if(backendServer == 1){

			UDPConnections(1);

			if (sendto(Asock, buf, MAXBUFLEN , 0, pA->ai_addr, pA->ai_addrlen) == -1){
			    perror("sendto 1 ");
			}
		}

		if( backendServer == 2){

			UDPConnections(2);
			
			if (sendto(Bsock, buf, MAXBUFLEN , 0, pB->ai_addr, pB->ai_addrlen) == -1){
				    perror("sendto 1 ");
			}

		}

	}

	memset(buf,0, sizeof *buf);


	sin_size = sizeof their_addr;

    if ((numbytes = recvfrom(udpsock, buf, MAXBUFLEN-1 , 0,
        (struct sockaddr *)&their_addr, &sin_size)) == -1) {
        perror("recvfrom");
    }
    if( backendServer == 1){
    	close(Asock);
    }
    if(backendServer == 2){
    	close(Bsock);
    }
    close(udpsock);
    

    buf[numbytes] = '\0';
    
}

int serverMain::SocketConnection(std::string protocol, const char * Port, bool Server){
	
	struct addrinfo *p, *servinfo;
	int yes=1;
	int tempsock;

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
        if ((tempsock = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if(Server == true ){
			if (setsockopt(tempsock,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof yes) == -1) {
			    perror("setsockopt");
			    return -1;
			}

			if (bind(tempsock, p->ai_addr, p->ai_addrlen) == -1){
				close(tempsock);
				perror("server:bind");
				continue;

			}
		}
		break;
	}

	freeaddrinfo(servinfo);

	if( p == NULL){

		fprintf(stderr, "server: failed to bind \n");
		return -1;
	}


	if(listen(tempsock,BACKLOG) == -1 && protocol.compare("TCP") == 0){

		perror("listen");
		return -1;

	}

	if( protocol.compare("TCP") == 0){

		sockfd = tempsock;
	}

	if ( protocol.compare("UDP") == 0){

		udpsock = tempsock;
	}

	return 0;

}


int main(void){
	

	int new_fd, numbytes;
	char buf[MAXDATASIZE];  // listen on sock_fd, new connection on new_fd
    struct sigaction sa;
    char s[INET6_ADDRSTRLEN];
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;


    serverMain SM;
	
    if( SM.SocketConnection("TCP", TCPPort, true) != 0){

    	std::cout<< "An Error has occured in the allocating a TCP socket"<<std::endl;
    }

    std::cout<< "The Main Server is up and running" << std::endl;

	sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;


    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

	char msgA[100];
	std::string Startup = "-*-";

	strcpy(msgA ,Startup.c_str());
	SM.SendAndRcv(1,1, msgA);

	std::cout<< "The Main Server has received the country list from Server A using UDP over port " << UDPPort << std::endl;
	std::string outputA(msgA);

	char msgB[100];
	strcpy(msgB ,Startup.c_str());
	SM.SendAndRcv(1,2, msgB);

	std::cout<< "The Main Server has received the country list from Server B using UDP over port " << UDPPort << std::endl;

	std::cout<< "Country -> Backend Server"<<std::endl;

	std::stringstream ssA(outputA);
	std::string countryA;

	while (std::getline(ssA, countryA, ',')){

		SM.allCountries[countryA] = 1;

		std::cout<< countryA << " -> A" <<std::endl;
	}
	//loop through and add the countries

	
	std::string outputB(msgB);

	std::stringstream ssB(outputB);
	std::string countryB;

	while (std::getline(ssB, countryB, ',')){

		SM.allCountries[countryB] = 2;
		std::cout<<countryB << " -> B" <<std::endl;
	}




    while(1) {  // main accept() loop
        
        sin_size = sizeof their_addr;
        new_fd = accept(SM.sockfd, (struct sockaddr *)&their_addr, &sin_size);
        
        if (new_fd == -1) {
            
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);


        if (!fork()) { // this is the child process

            close(SM.sockfd); // child doesn't need the listener
            
            memset(buf,0, sizeof *buf);

            if (send(new_fd, "Hello, world!", 13, 0) == -1){
                perror("send");
            }

            if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
		        perror("recv");
		        exit(1);
		    }
		    buf[numbytes] = '\0';

		    //Received Query!

		    //Check if valid and who to send to
		    
		    std::string output(buf);

		    std::string country = output.substr(0, output.find(',')-1);

			std::string tempid= output.substr(output.find(',') + 2, output.size());


			int userid = std::stoi(tempid);

			std::cout<< "The Main Server has received the request on User " <<userid << " in " << country<< " from the client using TCP over port " << TCPPort<<std::endl;
			int BackServer = SM.allCountries[country];
			bool flag = true;
			if(BackServer > 0){
				


				int BackServer = SM.allCountries[country];
				std::stringstream ss;

				SM.SendAndRcv(1, BackServer, buf);
				std::string serverString;
				std::string serverPort;
				if (BackServer == 1){
					serverString = "A";
					serverPort = APort;
				}
				if( BackServer == 2){
					serverString = "B";
					serverPort = BPort;
				}

				std::cout<< country << " shows up in "<<serverString<< std::endl;
				std::cout<< "The Main Server has sent request from User " <<userid<< " to server "<<serverString<< " using UDP over port "<<serverPort<< std::endl;
				
				std::string temp(buf);
				bool flag = true;
				if (temp.compare("-1") == 0){

					std::cout<< "The Main Server has received 'User ID: Not found' from server "<<serverString<<std::endl;
					output = "User " + tempid + " not found";

				}

				else if (temp.compare("-2") == 0){
					output = tempid+ " is the ONLY user in " + country;
				}

				else if (temp.compare("-3") == 0){
					output = tempid+ " is already connected to all other users, no new recommendation";
				}

				else{

					flag = false;

					output = "The client has recieved results from Main Server: Recommended new friend ID is " + temp + " to User ID " + tempid;
					
					

					std::cout<< "The Main Server has received a searching result of User "<< userid << " from server "<< serverString<<std::endl;

					const void * a = output.c_str();
				    if (send(new_fd, a, output.length() , 0) == -1){
				        perror("send");
				    }

				    std::cout<< "The Main Server has sent searching result(s) to client using TCP over port "<< TCPPort << std::endl;
				    std::cout<<std::endl;
				    close(new_fd);
				    exit(0);
				}

				const void * a = output.c_str();
				    if (send(new_fd, a, output.length() , 0) == -1){
				        perror("send");
				    }

			    std::cout<< "The Main Server has sent error to client using TCP over port "<< TCPPort <<std::endl; 
			    std::cout<<std::endl;
			    close(new_fd);
			    exit(0);

				
				

			}
			else{
				std::cout<< country << " does not show up in A & B " << std::endl;
				output = country + ": Not found";

				const void * a = output.c_str();
			    if (send(new_fd, a, output.length() , 0) == -1){
			        perror("send");
			    }
				std::cout<< "The Main Server has sent: " << output<< " to the client  using TCP over port " << TCPPort << std::endl;
				std::cout<<std::endl;
				
				close(new_fd);
			    exit(0);
			}

		    // Send Results to Client

		    close(new_fd);
		    exit(0);

		}

    }

    return 0;
}


