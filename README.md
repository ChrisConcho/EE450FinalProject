# EE450FinalProject


#Christian Concho
USCID: 1188708484
#Abstract

This project is a client communicating with a main server over TCP. The client sends a query that the main server will determine one of two backend servers to communicate with to determine the result over  UDP 

#File Details
client.h
	Holds information used in the TCP connections and a function to connect.

client.cpp
	Responsible for grabbing user input in order to create a query.
	Sends and Recieves messages over TCP  with mainserver

servermain.h
	SendAndRcv: sends and receives messages from backend server over UDP 
	UDPConnections: establishes the connection with the requested backend server (outgoing messages)
	SocketConnection: Creates sockets dedicated to listen on UDP port and TCP port (incoming messages)
	allCountries: responsible for mapping countries to the respected backend server

servermain.cpp
	Listens on TCP port for incoming client connections.
	Creates thread for child to help clients and resume back to listening on port.
	Child communicates queries from client to the respective backend server and forwards the results back to the client.

serverA.h and serverB.h
	c_ID_map: holds each map of userID's to re-indexed userID's for each country in a vector
	c_idx: maps country to the index on c_ID_map
	c_Matrix: 3D int array. 
		D1: country
		D2: user's freinds 
		D3: relation of neighbor to the user
		ex. user Chris and user Max live in country Canada
			c_Matrix[canada][Chris][Max] = 1 -> Chris and Max are friends in Canada.
			user i and user j live in country k
			c_Matrix[k][i][j] = 0 -> i and j are not friends in k.
	LoadData: parse data.txt to re-index user ID's and create Friend Relation Matrix (c_Matrix)
	SendUDP: Send a msg (the results) over UDP  to the main server (outgoing messages)
	FindFriends: Determines the best possible neighbor based on common friends.
	Constructor: Creates sockets for UDP connections to send messages to the main server
	Destructor: Free all allocated data needed to create c_Matrix and c_ID_map and close sockets.
serverA.cpp and serverB.cpp
	Establishes socket connection to listen on a dedicated port for all incoming queries from the main server
	listens on UDP port and creates a child to handle all main server queries
	Determines the best possible friend for the userID corresponding to the country they live in
	Returns the results over UDP to the main server



#How to send a query via client

You will be asked to put in your user ID and country. The user ID must come first followed by a single space then the country name. No spaces afterwards are allowed. Only the one space in between. No commas or characters in betweeen. 

#References:
	TCP and UDP connection code was inspired form http://www.beej.us/guide/bgnet/html/.









#MORE DETAILED DESCRIPTION BELOW


# ---------- Backend Server A/B ------------- server#.cpp server#.h -> # = A or B

# have at most 10 countries that are only in letters 
	#- read in line from data1.txt and check for letter to create new country
# read in id of user then there will be a space to seperate from its corresponding neighbors
	# - keep storing neighbors until we hit a new line character \n
# store each country in a Map such as adjacency matrix, adjacency list or Compressed Sparse Row (CSR) format

#recieving child process of main server query request check the following conditions
# is the user already connected to every other user -> return None
# is the user the only user in the map -> return None
# for every user (n)  NOT connected to the user check how many freinds they have in common
	# Prof. recommends using a set to find number of common neighbors
# if each n not connected to user (u) has no common friends recommend the n with the most friends
	# Tie breaker: multiple n's with same number of freinds then return the smallest ID (top of the matrix)
# recommend the n with the most friends in common with u
	# Tie breaker: same as previous tie breaker
# result sent back to main server using UDP connection

#UDP A port: 30484 
#UDP B port: 31484

# --------- Main Server ----------------------- servermain.cpp servermain.h

# create Server A and Server B
# have them read their data1.txts
# have them return which countries they are responsible for 
# use unordered map to store which countries belong to which server for fast access
# listen for client socket connections
# accept client socket connection and fork()

# check if country or user id or not found then print out a message 
# assign the correct backend server to find recommended user via UDP connection
# Send result back to client via a TCP connection

#UDP Port: 32484
#TCP Port: 33484

# ---------- Client -------------- client.cpp client.h

# on boot up ask client "Enter Country Name: " and " Enter User ID: "
# establish a TCP socket connection with the Main Server.
# listen for response (from child of main server) and print out message
# Immediately print( "-----Start a new request-----") and start again from the top