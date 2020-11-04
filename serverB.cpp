#include "serverB.h"





void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int bServerB::FindFriends(int userID, std::string country){

	int friendSuggestion;
	int countryidx = c_idx[country];
	//check if user exists in country
	std::map<int,int> tempMap = *c_ID_map[countryidx];

	if(tempMap.find(userID) == tempMap.end() ){
		// -1 means no userid  in map
		return -1;
	}


	//grab row of friends
	int idx = c_idx[country];
	int newidx = tempMap[userID];
	int* user_friends = c_Matrix[idx][newidx];


	
	//loop through once to see if they are friends with everyone
		//return none if true
	int total_friends =  0;
	if(tempMap.size() == 1){
		//The only user in this map
		return -2;
	}
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
	std::cout<< "The server B is searching possible friends for User " << userID << std::endl;

	std::vector<std::pair<int,int> > all_friendScores;
	for (unsigned int n_freinds = 0; n_freinds < tempMap.size(); n_freinds++){

		int similarity_score = 0;
		int total_friends = 0;
		if (n_freinds != newidx && user_friends[n_freinds] != 1){
			for( unsigned int j = 0 ; j < tempMap.size(); j++){
				if(c_Matrix[idx][n_freinds][j] == 1){
					if(user_friends[j]==1){
						similarity_score++;
					}
					total_friends++;
				}
			}
		}
		// std::cout<< "user: " << n_freinds<< " scored -> " << similarity_score<<std::endl;
		all_friendScores.push_back(std::make_pair(similarity_score,total_friends));
	}
	int max = all_friendScores[0].first;
	int max_idx = 0;
	// std::cout << "New Max -> " << max_idx << std::endl;
	int contender;
	for (int neighbor = 0; neighbor < all_friendScores.size(); neighbor++){
		contender = all_friendScores[neighbor].first;

		if (contender>max){
			// std::cout << "New Max -> " << neighbor << std::endl;
			max = contender;
			max_idx = neighbor;

		}

		if ( contender == max){
			if( total_friends == 0){
				if ( all_friendScores[neighbor].second > all_friendScores[max_idx].second){
					// std::cout << "New Max -> " << neighbor << std::endl;
					max = contender;
					max_idx = neighbor;
				}
			}

		}


	}
	std::map<int,int>::iterator it;

	for ( it = tempMap.begin(); it != tempMap.end(); it++){
		if (it->second == max_idx){
			friendSuggestion = it->first;
			break;
		}
	}

		//tally similar friends 
		// save score and id
	//if no similar friends at all or same amount for top contenders
		//return user with most friends in general
			//tieBreak: return user with smaller ID
		//
		//save a visted 
	//
	return friendSuggestion;
}

int bServerB::SendUDP( std::string msg_in){

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



int** bServerB::CreateAdjacencyMatrix(std::map<int,int> mapID, std::vector<std::vector<int> > vs){

	int** AdjacencyMatrix = new int* [mapID.size()];

	// std::cout<< "Dimensions of Matrix " << mapID.size() << " x " << mapID.size() << std::endl;

	for( unsigned int column = 0; column < mapID.size() ; column++){

		AdjacencyMatrix[column] = new int [mapID.size()];

		// std::cout<< "User " << column << ": [";

		for( unsigned int row = 0 ; row < mapID.size() ; row++){

			AdjacencyMatrix[column][row] = 0;

			// std::cout<< AdjacencyMatrix[column][row]<< " ";

		}
		// std::cout<< "]" << std::endl;
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

void bServerB::PrintMatrix(){

	std::map<std::string ,int >::iterator it;

		int i = 0;
		for (it = c_idx.begin(); it != c_idx.end(); it++){

			std::cout<< "Country: " << it->first<< std::endl;

			int dimensions = it->second;

			std::cout << "country idx: " << it->second <<std::endl;
			// int rows = sizeof c_Matrix[i] / sizeof c_Matrix[i][0];

			unsigned int rows = (c_ID_map[dimensions])->size();

			std::cout<< "rows: " << rows << std::endl;


			for ( unsigned int j= 0 ; j < rows; j++){
				
				// int columns = sizeof c_Matrix[i][j] /sizeof c_Matrix[i][j][0];
				unsigned int columns = (c_ID_map[dimensions])->size();
				std::cout<< "columns: " << columns << std::endl;
				// std::cout<< "Number of freinds: " << sizeof(*c_Matrix[i][j])<<std::endl;

				std::cout<< "user: " << j<< "[";
				for ( unsigned int k = 0 ; k < columns; k++){

					std::cout<<" " <<c_Matrix[i][j][k];
				}
				std::cout<< "]"<<std::endl;
			}
			std::cout<<std::endl;
			i++;
		}
}

void bServerB::LoadDataMap(std::string datatxt){

	std::ifstream input(datatxt);
	

	bool started = false;

	if ( input.is_open()){


		int re_idx = 0;
		int total_countries = 0;
		std::string line;
		std::vector<std::vector<std::vector<int> >* > AllvStreams;
		std::vector<std::vector<int> >* countryStreams = new std::vector<std::vector<int> >;
		std::map<int,int>* userToMatrix = new std::map<int,int>;
		std::string currentCountry;
		while(getline(input,line)){



			// we find a new row of user + friends
			if(std::isdigit(line.front())){


				std::stringstream ss;
				int userID;

				ss<<line;

				ss>>userID;
				
				(*userToMatrix)[userID] = re_idx;

				// std::cout<< "[" <<userID << "   ->   " <<re_idx <<" ]"<< std::endl;
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

					c_idx[currentCountry] = total_countries;

					total_countries++;

					AllvStreams.push_back(countryStreams);

					//AllMatrices[countriesVect.size()] = CreateAdjacencyMatrix(*userToMatrix, vstreams);

					//vstreams.clear();

					userToMatrix = new std::map<int,int>;

					countryStreams = new std::vector<std::vector<int> >;
					re_idx = 0;

				}

				else{
					started = true;

				}
				currentCountry = line;
				// std::cout<< "Re-Index Map of " << line << " User ID's" << std::endl;

			}

		}

		if(started){

			c_ID_map.push_back(userToMatrix);

			c_idx[currentCountry] = total_countries;

			AllvStreams.push_back(countryStreams);

			//AllMatrices[countriesVect.size()] = CreateAdjacencyMatrix(*userToMatrix, vstreams);

			//vstreams.clear();
		}


		c_Matrix = new int**[c_ID_map.size()];

		int country_num = 0;

		for (int i = 0 ; i < c_ID_map.size(); i++){


			std::map<int,int> uTm = *(c_ID_map[i]);
			// std::cout<<"Creating Matrix for country#: " << country_num << std::endl;

			c_Matrix[country_num] = CreateAdjacencyMatrix(uTm, *AllvStreams[country_num]);

			delete AllvStreams[country_num];

			country_num++;
		}
		//finsihed going through all the data

		
		
	}


}


int main(void){
	
	// std::cout<<"Starting Server B \n";
	int numbytes, udpsock, Bsock;
	char buf[MAXDATASIZE];  // listen on sock_fd, new connection on new_fd
    struct sigaction sa;
    char s[INET6_ADDRSTRLEN];
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;

    bServerB BSB;
    // std::cout<<"Loading Data.txt\n";
    BSB.LoadDataMap("testcases/testcase3/data2.txt");


	
    // std::cout<<"starting UDP Socket Connection\n";
    Bsock = SocketConnection("UDP", BPort, true);

    std::cout<<"Server B is up and running on port " << BPort<<std::endl;
    std::map<std::string ,int >::iterator it;

    std::string msg_countryList;
    for ( it = BSB.c_idx.begin() ; it != BSB.c_idx.end(); it++){

    	msg_countryList += it->first + "," ;
    }
    msg_countryList.pop_back();
    

  	
    // BSB.PrintMatrix(); -------------------- SEGFAULTS

	while(1){


	    sin_size = sizeof their_addr;
	    if ((numbytes = recvfrom(Bsock, buf, MAXBUFLEN-1 , 0,
	        (struct sockaddr *)&their_addr, &sin_size)) == -1) {
	        perror("recvfrom");
	        exit(1);
	    }

	    if(!fork()){
	    	buf[numbytes] = '\0';
		    // printf("listener: packet contains \"%s\"\n", buf);


		    std::string output(buf);

		    if(output.compare("-*-") == 0){

		    	BSB.SendUDP(msg_countryList);
		    	std::cout<<"Server B has sent country list to Main Server "<<std::endl;

		    	exit(0);
		    }


		    std::cout<< output<<std::endl;

		    std::string user_country = output.substr(0, output.find(',')-1);


			std::string tempid= output.substr(output.find(',') + 2, output.size());

			int userID = std::stoi(tempid);

		    int friends;

		    std::cout<<"Server B has received request for finding possible friends of User "<< userID << " in " << user_country<<std::endl;

		    friends = BSB.FindFriends(userID,user_country);
	   		if( friends == -1){
	   			std::cout<< "User " <<userID << " does not show up in "<< user_country<<std::endl;
	   			BSB.SendUDP("-1");
	   			std::cout<< "The server B has sent 'User "<<userID<< " not found' to MainServer"<<std::endl;
	   		}
	   		else if( friends == -2){
	   			std::cout<< "There are no other people in this map but you"<<std::endl;
	   			BSB.SendUDP("-2");
	   			std::cout<< "The server B has sent ' No possible friends' to the Main Server"<<std::endl;
	   		}
	   		else if(friends == -3){

	   			std::cout<< "This userID is friends with Everyone!"<<std::endl;
	   			BSB.SendUDP("-3");
	   			std::cout<< "The Server B has sent ' No new friends to be made' to the Main Server"<<std::endl;
	   		}
	   		else{
	   			std::string msg_Results =std::to_string(friends);
	   			std::cout<< "Recommend new friend " + msg_Results + " to user " << userID << std::endl;
	   			BSB.SendUDP(msg_Results);
	   		}
	   		exit(0);
	    }

	    

	}
 }
