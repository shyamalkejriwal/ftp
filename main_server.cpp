#ifndef SERVER_H
#define SERVER_H
#include "server.hpp"
#endif

int main(int argc, char *argv[])
{
	server s1(SPORT,BACKLOG);
	s1.serve();
	return 0;
}