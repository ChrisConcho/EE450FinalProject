all: serverA serverB servermain client
serverA: serverA.h serverA.cpp
	g++ -std=c++11 -o serverA serverA.cpp -v

serverB: serverB.h serverB.cpp
	g++ -std=c++11 -o serverB serverB.cpp -v

servermain: servermain.h servermain.cpp
		g++ -std=c++11 -o servermain servermain.cpp -v

client: client.h client.cpp
	g++ -std=c++11 -o client client.cpp -v