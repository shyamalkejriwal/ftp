#ifndef CONNECT_H
#define CONNECT_H
#include "connect.hpp"
#endif
#include "reply.hpp"

#define SPORT "5000"

class server : public connection{
	char command[MAXSIZE];
	char *dataport;

	void eput();
	void eget();
	void els();
	void ecd();
	void epwd();
	void equit();
	void eport();
	bool send_reply(const char *s);
	void serve_client();
public:
	server();
	server(int blog);
	server(const char *port, int blog);
	void serve();
};