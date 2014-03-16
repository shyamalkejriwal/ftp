#ifndef CONNECT_H
#define CONNECT_H
#include "connect.hpp"
#endif
#include "reply.hpp"

#define CPORT "4000"
#define MAXTRY 10

class client : public connection{
	sockdesc ctrlsd;
	const char *server_IP;
	const char *server_port;
	char command[MAXSIZE];

	bool get_reply();
	bool get_reply(int code);
	bool eport();
	void eput();
	void eget();
	void els();
	void ecd();
	void epwd();
	void equit();
	void elcd();
	void elcmd();
public:
	client(const char *IP, const char *port);
	void get_service();
};