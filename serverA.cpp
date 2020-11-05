#include<iostream>

#define BACKLOG 10
#define UDPPort "32484"
#define APort "30484"
#define MAXDATASIZE 100
#define localhost "127.0.0.1"
#define MAXBUFLEN 100

#include "serverA.h"


void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int bServerA::FindFriends(int userID, std::string country){


	// this wil be our solution we return
	int friendSuggestion;
	int countryidx = c_idx[country];
	//check if user exists in country
	std::map<int,int> tempMap = *c_ID_map[countryidx];
	std::map<int,int> trueID = *ridx_id_map[countryidx];

	if(tempMap.find(userID) == tempMap.end() ){
		// -1 means no userid  in map
		return -1;
	}


	//grab row of friends
	int idx = c_idx[country];
	int newidx = tempMap[userID];
	int* user_friends = c_Matrix[idx][newidx];


	if(tempMap.size() == 1){
		//The only user in this map
		return -2;
	}



	int total_friends =  0;

	//loop through once to see if they are friends with everyone
	for( unsigned int j = 0 ; j < tempMap.size(); j++){

		if ( user_friends[j] == 1){
			total_friends++;
		}
	}

	if ( total_friends == tempMap.size()-1){
		// -2 means they are friends with EVERYONE
		return -3;
	}


	//loop through all possible friends in map
	std::cout<< "The server A is searching possible friends for User " << userID << std::endl;

	std::vector<std::pair<int,int> > all_friendScores;
	for (unsigned int n_freinds = 0; n_freinds < tempMap.size(); n_freinds++){

		int similarity_score = 0;
		int total_friends = 0;
		//check if the neigbor is themself and that they are not already friends
		if (n_freinds != newidx && user_friends[n_freinds] != 1){
			//loop through friends list to see similarity score
			for( unsigned int j = 0 ; j < tempMap.size(); j++){
				if(c_Matrix[idx][n_freinds][j] == 1){
					if(user_friends[j]==1){
						//both have the same friend
						similarity_score++;
					}
					//keep track of total friends for tie breaks scenario
					total_friends++;
				}
			}
		}
		//add possible friends to a vector so we can compare later
		all_friendScores.push_back(std::make_pair(similarity_score,total_friends));
	}
	//set max to first user for initialization

	int max = all_friendScores[0].first;
	int max_idx = 0;

	int contender;
	// loop through all possible friends and evaluate best option with similarity scores calculated.
	for (int neighbor = 0; neighbor < all_friendScores.size(); neighbor++){
		contender = all_friendScores[neighbor].first;
		
		//compare similarity score
		if (contender>max){
			max = contender;
			max_idx = neighbor;
		}
		// tie breaker on same scores
		if ( contender == max){
			//if client has No friends then we compare who has most total friends
			if( total_friends == 0){
				if ( all_friendScores[neighbor].second > all_friendScores[max_idx].second){

					max = contender;
					max_idx = neighbor;
				}
			}
			//otherise we check who has the smaller original id
			else{
				if( trueID[max_idx] > trueID[neighbor]){
					max = contender;
					max_idx = neighbor;
				}
			}

		}
	}
	//return the neigbor's original id to the client.
	friendSuggestion = trueID[max_idx];
	return friendSuggestion;
}

int bServerA::SendUDP( std::string msg_in){

    const void * msg = msg_in.c_str();

    if (sendto(udpsock, msg, msg_in.length() , 0, p->ai_addr, p->ai_addrlen) == -1){
    	perror("send");
    }

	return 0;


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

int** bServerA::CreateAdjacencyMatrix(std::map<int,int> mapID, std::vector<std::vector<int> > vs){

	int** AdjacencyMatrix = new int* [mapID.size()];

	for( unsigned int column = 0; column < mapID.size() ; column++){

		AdjacencyMatrix[column] = new int [mapID.size()];


		for( unsigned int row = 0 ; row < mapID.size() ; row++){

			AdjacencyMatrix[column][row] = 0;

		}
	}

	int column = 0;

	for ( int i = 0 ; i < vs.size() ; i++){

		std::vector<int> row = vs[i];

		int friendID;

		for ( int j = 0 ; j < row.size(); j++){

			friendID = row[j];

			AdjacencyMatrix[column] [mapID[friendID]] = 1;
		}

		column++;
	}

	return AdjacencyMatrix;

}

void bServerA::PrintMatrix(){

	std::map<std::string ,int >::iterator it;

		int i = 0;
		for (it = c_idx.begin(); it != c_idx.end(); it++){

			std::cout<< "Country: " << it->first<< std::endl;

			unsigned int rows = (c_ID_map[it->second])->size();

			for ( unsigned int j= 0 ; j < rows; j++){
				
				unsigned int columns = (c_ID_map[it->second])->size();

				std::cout<< "[";
				for ( unsigned int k = 0 ; k < columns; k++){

					std::cout<<" " <<c_Matrix[i][j][k];
				}
				std::cout<< "]"<<std::endl;
			}
			std::cout<<std::endl;
			i++;
		}
}

void bServerA::LoadDataMap(std::string datatxt){

	std::ifstream input(datatxt);
	

	bool started = false;

	if ( input.is_open()){


		int re_idx = 0;
		int total_countries = 0;
		std::string line;
		
		std::vector<std::vector<std::vector<int> >* > AllvStreams;
		std::vector<std::vector<int> >* countryStreams = new std::vector<std::vector<int> >;
		
		std::map<int,int>* userToMatrix = new std::map<int,int>;
		std::map<int,int>* reIndexToOG = new std:: map <int, int>;
		
		std::string currentCountry;
		while(getline(input,line)){
			// we find a new row of user + friends
			if(std::isdigit(line.front())){

				std::stringstream ss;
				int userID;

				ss<<line;

				ss>>userID;
				
				(*userToMatrix)[userID] = re_idx;
				(*reIndexToOG)[re_idx] = userID;

				re_idx++;

				int temp;

				std::vector<int> row;

				while( ss>> temp){
					row.push_back(temp);
				}

				countryStreams->push_back(row);
			}

			else{

				if(started){

					c_ID_map.push_back(userToMatrix);

					ridx_id_map.push_back(reIndexToOG);

					c_idx[currentCountry] = total_countries;

					total_countries++;

					AllvStreams.push_back(countryStreams);


					userToMatrix = new std::map<int,int>;

					reIndexToOG = new std:: map <int, int>;

					countryStreams = new std::vector<std::vector<int> >;
					re_idx = 0;

				}

				else{
					started = true;

				}
				currentCountry = line;
			}

		}

		if(started){

			c_ID_map.push_back(userToMatrix);

			ridx_id_map.push_back(reIndexToOG);

			c_idx[currentCountry] = total_countries;

			AllvStreams.push_back(countryStreams);

		}
		c_Matrix = new int**[c_ID_map.size()];

		int country_num = 0;

		for (int i = 0 ; i < c_ID_map.size(); i++){

			std::map<int,int> uTm = *(c_ID_map[i]);

			c_Matrix[country_num] = CreateAdjacencyMatrix(uTm, *AllvStreams[country_num]);

			delete AllvStreams[country_num];

			country_num++;
		}
		//finsihed going through all the data		
	}

}


int main(void){
	
	int numbytes, udpsock, Asock;
	char buf[MAXDATASIZE];  // listen on sock_fd, new connection on new_fd
    struct sigaction sa;
    char s[INET6_ADDRSTRLEN];
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    
    bServerA BSA;

    BSA.LoadDataMap("testcases/testcase3/data1.txt");

    Asock = SocketConnection("UDP", APort, true);

    std::cout<<"Server A is up and running on port " << APort<<std::endl;
    std::map<std::string ,int >::iterator it;

    std::string msg_countryList;
    for ( it = BSA.c_idx.begin() ; it != BSA.c_idx.end(); it++){

    	msg_countryList += it->first + "," ;
    }
    msg_countryList.pop_back();
    


	while(1){


	    sin_size = sizeof their_addr;
	    if ((numbytes = recvfrom(Asock, buf, MAXBUFLEN-1 , 0,
	        (struct sockaddr *)&their_addr, &sin_size)) == -1) {
	        perror("recvfrom");
	        exit(1);
	    }

	    if(!fork()){
	    	buf[numbytes] = '\0';

		    std::string output(buf);

		    if(output.compare("-*-") == 0){

		    	BSA.SendUDP(msg_countryList);

		    	std::cout<<"Server A has sent country list to Main Server "<<std::endl<<std::endl;

		    	exit(0);
		    }

		    std::string user_country = output.substr(0, output.find(',')-1);

			std::string tempid= output.substr(output.find(',') + 2, output.size());

			int userID = std::stoi(tempid);

		    int friends;

		    std::cout<<"Server A has received request for finding possible friends of User "<< userID << " in " << user_country<<std::endl;

		    friends = BSA.FindFriends(userID,user_country);
	   		if( friends == -1){
	   			std::cout<< "User " <<userID << " does not show up in "<< user_country<<std::endl;
	   			BSA.SendUDP("-1");
	   			std::cout<< "The server A has sent 'User "<<userID<< " not found' to MainServer"<<std::endl;
	   		}
	   		else if( friends == -2){
	   			std::cout<< "There are no other people in this map but you"<<std::endl;
	   			BSA.SendUDP("-2");
	   			std::cout<< "The server A has sent ' No possible friends' to the Main Server"<<std::endl;

	   		}
	   		else if(friends == -3){

	   			std::cout<< "This userID is friends with Everyone!"<<std::endl;
	   			BSA.SendUDP("-3");
	   			std::cout<< "The Server A has sent ' No new friends to be made' to the Main Server"<<std::endl;
	   		}
	   		else{
	   			std::string msg_Results =std::to_string(friends);
	   			std::cout<< "Recommend new friend " + msg_Results + " to user " << userID << std::endl;
	   			BSA.SendUDP(msg_Results);

	   			std::cout<< "The server A has sent the results to the Main Server" << std::endl;
	   		}
	   		// to separate and easier to read
	   		std::cout<<std::endl;
	   		exit(0);
	    }
	    
	  
	}
 }

