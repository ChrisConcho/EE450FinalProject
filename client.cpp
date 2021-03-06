
#include "client.h"


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int client::TCPConnection(){
    //clear hints in case of unwanted data
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    //get address information
    if ((rv = getaddrinfo(localhost, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }
        // connect to the socket
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }



    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);

    

    freeaddrinfo(servinfo); // all done with this structure

    //recieve the information back from the main server
    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }
    buf[numbytes] = '\0';
    return 0;
}

int main(int argc, char *argv[])
{
    //create a client
    client c1; 
    std::cout<< "The client is up and running. \n"; 
    char buf[MAXDATASIZE];

    //main while loop to continue to make queries
while(1){
        //start the connection
        if(c1.TCPConnection() == 0){
            int numbytes;
            //request information from the user
            std::cout<< "Please enter the User ID: "<< std::endl;
            std::cout<< "Please enter the Country Name: "<< std::endl;
            int userID;
            std::cin>>userID;
            std::string countryName;

            std::cin>>countryName;
            // create a query
            std::string query =  countryName + " , " + std::to_string(userID);
            const void * a = query.c_str();
            // send the query to the main server 
            if (send(c1.sockfd, a ,query.length(), 0) == -1){
                        perror("send");
                    }
            // recieve the results from the main server
            std::cout<< std::endl<<"The client has sent User "<<userID <<" and "<<countryName<<" to Main Server using TCP" << std::endl;
            if ((numbytes = recv(c1.sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
                perror("recv");
                exit(1);
            }
            // add delimiting char
            buf[numbytes] = '\0';
            std::string output(buf);
            std::cout<<std::endl;
            // pring out the results
            std::cout<< output << std::endl;
            // close sock becuase query is complete
            close(c1.sockfd);
            // clear the buffer
            memset(buf,0, sizeof *buf);
            std::cout<<std::endl<<"-------Start a new request------ \n";
        }
    }

    

    return 0;
}