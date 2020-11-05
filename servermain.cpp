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
    // clear hints
	memset(&hints, 0, sizeof hints);


	hints.ai_family = AF_UNSPEC;
	
	hints.ai_socktype = SOCK_DGRAM;
	
	hints.ai_flags = AI_PASSIVE; 
	
	// determine which back end server we are connecting to 
	if ( backendServer == 1){

		rv = getaddrinfo(localhost, APort, &hints, &servinfoA);
	}
	if( backendServer == 2){

		rv = getaddrinfo(localhost, BPort, &hints, &servinfoB);
	}

	if (rv != 0 ) {

		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
       
	}
	// server A 
	if(backendServer == 1){

		for(pA = servinfoA; pA != NULL; pA = pA->ai_next) {
			// create a socket 
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
    // server B 
    if(backendServer == 2)
    {
    	for(pB = servinfoB; pB != NULL; pB = pB->ai_next){
	    	// create a socket
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
	// create a UDP connection on dedicated port 
	if ( SocketConnection("UDP", UDPPort, true) != 0) {
    	std::cout<< "An Error has occured in the allocating a UDP socket"<<std::endl;
    	exit(1);
    }

    // send a msg to the backend server 
	if (send == 1){

		if(backendServer == 1){
			// server A UDP connection
			UDPConnections(1);
			// send query
			if (sendto(Asock, buf, MAXBUFLEN , 0, pA->ai_addr, pA->ai_addrlen) == -1){
			    perror("sendto 1 ");
			}
		}

		if( backendServer == 2){
			// server B UDP connection
			UDPConnections(2);
			// send query
			if (sendto(Bsock, buf, MAXBUFLEN , 0, pB->ai_addr, pB->ai_addrlen) == -1){
				    perror("sendto 1 ");
			}

		}

	}
	// clear buffer 
	memset(buf,0, sizeof *buf);


	sin_size = sizeof their_addr;
	// listen on servermain dedicated udp socket for msg from backend server
    if ((numbytes = recvfrom(udpsock, buf, MAXBUFLEN-1 , 0,
        (struct sockaddr *)&their_addr, &sin_size)) == -1) {
        perror("recvfrom");
    }
    //close sockets
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
	//comments for this function on serverB.cpp (same function)
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

    //create a the main server
    serverMain SM;
	// start the TCP connection on dedicated port
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
	//special msg to denote request of country lists
	std::string Startup = "-*-";

	strcpy(msgA ,Startup.c_str());
	// send msg to server A
	SM.SendAndRcv(1,1, msgA);

	std::cout<< "The Main Server has received the country list from Server A using UDP over port " << UDPPort << std::endl;
	std::string outputA(msgA);

	char msgB[100];
	strcpy(msgB ,Startup.c_str());
	//send msg to Server B
	SM.SendAndRcv(1,2, msgB);

	std::cout<< "The Main Server has received the country list from Server B using UDP over port " << UDPPort << std::endl;

	std::cout<< "Country -> Backend Server"<<std::endl;

	std::stringstream ssA(outputA);
	std::string countryA;
	//print out the countries A is responsible for 
	while (std::getline(ssA, countryA, ',')){
		// store country in dictionary for quick look up
		SM.allCountries[countryA] = 1;

		std::cout<< countryA << " -> A" <<std::endl;
	}

	
	std::string outputB(msgB);

	std::stringstream ssB(outputB);
	std::string countryB;
	// print out countries B is responsible for
	while (std::getline(ssB, countryB, ',')){
		// sotre country in dictionary for quick look up
		SM.allCountries[countryB] = 2;
		std::cout<<countryB << " -> B" <<std::endl;
	}




    while(1) {  // main accept() loop
        
        sin_size = sizeof their_addr;
        // accept socket connection from client 
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
            	
            memset(buf,0, sizeof *buf);	//clear buffer

            if (send(new_fd, "Hello, world!", 13, 0) == -1){	// make sure they are able to receive msgs 
                perror("send");
            }

            if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {	// receive query from client 
		        perror("recv");
		        exit(1);
		    }
		    buf[numbytes] = '\0';	// add delimiter to msg 

		    //Received Query!

		    //Check if valid and who to send to
		    
		    std::string output(buf);

		    std::string country = output.substr(0, output.find(',')-1);	// grab country from query

			std::string tempid= output.substr(output.find(',') + 2, output.size());	// grab id from query


			int userid = std::stoi(tempid);

			std::cout<< "The Main Server has received the request on User " <<userid << " in " << country<< " from the client using TCP over port " << TCPPort<<std::endl;
			int BackServer = SM.allCountries[country];	// determine which server is repsonible for this country
			bool flag = true;
			if(BackServer > 0){	// check if country is dedicated to a server or not
				


				int BackServer = SM.allCountries[country];
				std::stringstream ss;
				// send query to the Backend server and receive results
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
				//store buf msg in temp to parse results
				std::string temp(buf);
				bool flag = true;
				if (temp.compare("-1") == 0){	// user not found

					std::cout<< "The Main Server has received 'User ID: Not found' from server "<<serverString<<std::endl;
					output = "User " + tempid + " not found";

				}

				else if (temp.compare("-2") == 0){	// no other users in map
					output = tempid+ " is the ONLY user in " + country;
				}

				else if (temp.compare("-3") == 0){	// user is friends with everyone
					output = tempid+ " is already connected to all other users, no new recommendation";
				}

				else{	// we have an actual friend suggestion

					flag = false;

					output = "The client has recieved results from Main Server: Recommended new friend ID is " + temp + " to User ID " + tempid;
					
					

					std::cout<< "The Main Server has received a searching result of User "<< userid << " from server "<< serverString<<std::endl;

					const void * a = output.c_str();
				    if (send(new_fd, a, output.length() , 0) == -1){	// send friend suggestion to the client 
				        perror("send");
				    }

				    std::cout<< "The Main Server has sent searching result(s) to client using TCP over port "<< TCPPort << std::endl;
				    std::cout<<std::endl;
				    close(new_fd);	//close socket and kill child 
				    exit(0);
				}
				// if no result and we have one of the error msgs above we send the error to the client 
				const void * a = output.c_str();
				    if (send(new_fd, a, output.length() , 0) == -1){
				        perror("send");
				    }

			    std::cout<< "The Main Server has sent error to client using TCP over port "<< TCPPort <<std::endl; 
			    std::cout<<std::endl;
			    close(new_fd);	//close socket 
			    exit(0);

				
				

			}
			else{	//no back end server responsible for the country queried 
				std::cout<< country << " does not show up in A & B " << std::endl;
				output = country + ": Not found";

				const void * a = output.c_str();
			    if (send(new_fd, a, output.length() , 0) == -1){	// send to the client 
			        perror("send");
			    }
				std::cout<< "The Main Server has sent: " << output<< " to the client  using TCP over port " << TCPPort << std::endl;
				std::cout<<std::endl;
				
				close(new_fd);	// close socket
			    exit(0);
			}

		    // Send Results to Client

		    close(new_fd);	// precautionary close socket
		    exit(0);

		}

    }

    return 0;
}


