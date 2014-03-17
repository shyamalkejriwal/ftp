#ifndef CONNECT_H
#define CONNECT_H
#include "connect.hpp"
#endif
#include "reply.hpp"

#define CPORT "4000"
#define MAXTRY 10		

//client class inherits connection class
class client : public connection{
	sockdesc ctrlsd;	//socket desc for control transfer
	const char *server_IP;		//server IP
	const char *server_port;	//server port
	char command[MAXSIZE];		//command input from user

	bool get_reply();	//receive a reply from server
	bool get_reply(int code);	//receive a reply and match if intended
	bool eport();	//send listener port to server
	void eput();	//send a file
	void eget();	//receive a file
	void els();		//ls on server
	void ecd();		//cd on server
	void epwd();	//pwd on server
	void equit();	//quit session
	void elcd();	//cd on local host
	void elcmd();	//terminal command on local host
public:
	client(const char *IP, const char *port);
	void get_service();	//connect, input commands and execute
};