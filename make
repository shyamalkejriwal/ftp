
all: server myftp
	
server: 
	g++ server.cpp connect.cpp main_server.cpp -o server

myftp: 
	g++ client.cpp connect.cpp main_client.cpp -o myftp

clean:
	rm server myftp