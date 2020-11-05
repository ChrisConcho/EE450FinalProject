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
#include <map>
#include<sstream>
#include<fstream>

class serverMain {

	public:
		serverMain(){

		}
		~serverMain(){

			freeaddrinfo(servinfoA);
			freeaddrinfo(servinfoB);
			close(sockfd);
		}
		std::map<std::string, int> allCountries;

		void SendAndRcv(int send, int backendServer, char buf[]);
		void UDPConnections(int backendServer);
		int SocketConnection(std::string protocol, const char * Port, bool Server);
		int sockfd;
		int udpsock;
		

		int Asock, Bsock;
		struct addrinfo hints, *servinfoA, *servinfoB, *pA, *pB;
	    socklen_t sin_size;
	    struct sigaction sa;
	    int rv;


};	