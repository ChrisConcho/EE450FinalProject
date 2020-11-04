all: serverA serverB servermain client
serverA: serverA.h serverA.cpp
	g++ -o serverA serverA.cpp -v

serverB: serverB.h serverB.cpp
	g++ -o serverB serverB.cpp -v

servermain: servermain.h servermain.cpp
		g++ -o servermain servermain.cpp -v

client: client.h client.cpp
	g++ -o client client.cpp -v