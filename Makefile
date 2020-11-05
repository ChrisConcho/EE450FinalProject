all: serverA.o serverB.o servermain.o client

serverA.o: serverA.h serverA.cpp
	g++ -std=c++11 -o serverA.o serverA.cpp -v

serverB.o: serverB.h serverB.cpp
	g++ -std=c++11 -o serverB.o serverB.cpp -v

servermain.o: servermain.h servermain.cpp
		g++ -std=c++11 -o servermain.o servermain.cpp -v

client: client.h client.cpp
	g++ -std=c++11 -o client client.cpp -v

clean:
	rm *.o client

serverA:
	./serverA.o
serverB:
	./serverB.o
servermain:
	./servermain.o
