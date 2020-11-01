# EE450FinalProject

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