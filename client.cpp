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

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
    int sockfd, numbytes, new_fd;  
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    int yes=1;

    int rv;
    char s[INET6_ADDRSTRLEN];

    if (argc != 1) {
        fprintf(stderr,"Too many arguments.");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;



    // getsock_check=getsockname(PORT,(struct sockaddr *)&my_addr,(socklen_t *)&addrlen);//Error checking
    // if (getsock_check== -1) {
        
    //     perror("getsockname"); exit(1);
    // }

    if ((rv = getaddrinfo("localhost", PORT, &hints, &servinfo)) != 0) {
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
    printf("client: connecting to %s\n", s);

    printf("Client is up and running." );

    freeaddrinfo(servinfo); // all done with this structure


    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }
    buf[numbytes] = '\0';
    printf("client: received '%s'\n",buf);

    std::cout<< "Please enter the User ID: "<< std::endl;
    std::cout<< "Please enter the Country Name: "<< std::endl;
    int userID;
    std::cin>>userID;
    std::string countryName;

    std::cin>>countryName;
    std::string query = countryName + " , " + std::to_string(userID);
    const void * a = query.c_str();
    if (send(sockfd, a ,query.length(), 0) == -1){
                perror("send");
            }
    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }
    buf[numbytes] = '\0';
    std::string output(buf);
    std::string user1 = output.substr(0,output.find(','));
    std::string user2 = output.substr(output.find(',')+1);
    printf("Client1 has received results from the Main Server: User %s , User %s is/are possible friend(s) of User %d in %s \n",user1.c_str(),user2.c_str(), userID, countryName.c_str());
    

    close(sockfd);

    return 0;
}