#ifndef CLIENT_H
#define CLIENT_H
#include "client.hpp"
#endif

int main(int argc, char *argv[])
{
	if(argc < 3){
		cerr << "Usage: myftp server-host-name server-port-name" << endl;
		exit(1);
	}
	client c1(argv[1], argv[2]);
	c1.get_service();
	return 0;
}