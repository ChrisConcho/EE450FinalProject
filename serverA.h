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

#include<iostream>
#include<fstream>
#include<sstream>
#include <vector>
#include <map>



class bServerA{


	public:
		std::vector<std::map<int,int>* > c_ID_map;

		std::map<std::string, int> c_idx;

		std::vector<std::map<int,int>*> ridx_id_map;
    	
    	int*** c_Matrix;

    	int udpsock;

    	struct addrinfo hints, *servinfo, *p;

		int rv;
    	
    	void LoadDataMap(std::string datatxt);
    	
    	int** CreateAdjacencyMatrix(std::map<int,int> mapID, std::vector<std::vector<int> > vs);
    	
    	void PrintMatrix();
    	
    	int SendUDP( std::string msg_in);

    	int FindFriends(int userID, std::string country);

    	bServerA(){

			memset(&hints, 0, sizeof hints);


			hints.ai_family = AF_UNSPEC;
			
			hints.ai_socktype = SOCK_DGRAM;
			
			hints.ai_flags = AI_PASSIVE; 
			


			rv = getaddrinfo(localhost, UDPPort, &hints, &servinfo);

			if (rv != 0 ) {

				fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
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
			}


    	};
		~bServerA(){
			if(!c_ID_map.empty()){
				

				for (int i = 0 ; i < c_ID_map.size() ; i++){

					delete c_ID_map[i];

					delete ridx_id_map[i];

				}


		}
		int matrixes = sizeof c_Matrix / sizeof c_Matrix[0];
		for (int i = 0 ; i <= matrixes ; i++ ){

			int rows = sizeof c_Matrix[i] / sizeof c_Matrix[i][0];

			for ( int j= 0 ; j <= rows; j++){


				delete c_Matrix[i][j];

			}

			delete c_Matrix[i];
		}
		delete c_Matrix;

		close(udpsock);
		freeaddrinfo(servinfo);
	}
};