#ifndef CONNECT_H
#define CONNECT_H
#include "connect.hpp"
#endif
#include "reply.hpp"

#define SPORT "5000"	//default control port the clients connect to

//server class inherits connection class
class server : public connection{
	char command[MAXSIZE];	//command received from client
	char *dataport;			//dataport sent by client for data transfer

	void eput();	//put command
	void eget();	//get command
	void els();		//ls command
	void ecd();		//cd command
	void epwd();	//pwd command
	void equit();	//quit command
	void eport();	//port command
	bool send_reply(const char *s);	//to send a reply to client on control port
	void serve_client();			//child process starts from here to serve a client
public:
	server();
	server(int blog);				//blog used in constructor for connection class
	server(const char *port, int blog);	//port and blog used in constructor for connection class
	void serve();	//parent server process runs here
};