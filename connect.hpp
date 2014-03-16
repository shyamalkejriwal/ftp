#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/wait.h>
#include <signal.h>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>

using namespace std;

typedef int sockdesc;   //socket descriptor

#define BACKLOG 10      //Maximum number of allowed backlog connections
#define MAXSIZE 1000

class connection{
	int ecode;			//error code
	char *lport;		//local listener port
	int backlog;		//Number of allowed backlog connections
protected:
	sockdesc listener;	//listener socket descriptor
	sockdesc newsd;		//socket descriptor for communication on most recent connection
	char *remoteIP;		//Remote IP to which a peer/server connected most recently
public:
	connection();
	connection(int blog);
	connection(const char *port, int blog);
	void set_lport(const char *port);
	void *get_in_addr(struct sockaddr *sa);			//get sockaddr, IPv4 or IPv6
	sockdesc get_listener_sock();					//get listener socket
	sockdesc accept_cnct();							//accept a pending request for connection by a peer
	sockdesc req_cnct(const char *ip, const char *port);	//request connection on a peer or server
	void prerror(int code);
	char *get_remoteIP();
};

class myftp_data{
	FILE *fp;
	sockdesc sd;
	size_t nbytes;		//number of bytes last sent/received
public:
	myftp_data(FILE *f, sockdesc s);
	bool send_file();
	bool recv_file();
	size_t getsize();
};


