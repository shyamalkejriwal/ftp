#ifndef SERVER_H
#define SERVER_H
#include "server.hpp"
#endif

#include "parse.cpp"	//conatins functions to parse command arguments 

void sigchld_handler(int s){
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

server::server() : connection(){
	dataport = NULL;
}

//blog used in constructor for connection class
server::server(int blog) : connection(blog){
	dataport = NULL;
}

//port and blog used in constructor for connection class
server::server(const char *port, int blog) : connection(port,blog){
	dataport = NULL;
}

//to send a reply to client on control port
bool server::send_reply(const char *s){
	if(send(newsd, (const void *)s, strlen(s)+1, 0) == -1){
    	perror("Reply fails");
    	return false;
	}
	return true;
}

//put command
void server::eput(){
	int argc;	//number of arguments
	char **argv, *fname;	//argv is array of arguments, fname is filename
	argc = pargs(command, &argv); //parse command to get arguments
	if(argc == 1){	//the only arg is put
		send_reply(_TFARG); //too few arguments
		return;
	}
	else if(argc == 2) fname = argv[1];	//user inputs only local fname, local fname becomes server fname
	else fname = argv[2];	//user inputs server fname
	if(access(fname, F_OK) != -1){	//file with fname already exists, can't be overwritten	
    	send_reply(_FXTS);
    	return;
	}

	FILE *f = fopen(fname, "wb");
	if(!f){	//file couldn't be opened
		send_reply(_FNOP);
		return;
	}

	sockdesc s;	
	if((s = req_cnct((const char *)remoteIP, (const char *)dataport)) < 0){ //server tries to connect to client on received dataport
		send_reply(_PUT);	//connection unsuccessful
		return;
	}
	if(!send_reply(rPUT)) return;	//connected, server ready to receive data
	
	myftp_data md(f, s);	//myftp_data obj to handle file transfer
	if(md.recv_file()) send_reply(PUT); //file successfully received by server
	else send_reply(_PUT);	//file transfer unsuccessful
	fclose(f);
	close(s);
}

//get command
void server::eget(){	//check eput() for comments
	int argc;
	char **argv, *fname;
	argc = pargs(command, &argv);
	if(argc == 1){
		send_reply(_TFARG);
		return;
	}
	
	fname = argv[1];	//file requested by client
	if(access(fname, F_OK) == -1){	//file doesn't exist
    	send_reply(_FNXTS);
    	return;
	}

	FILE *f = fopen(fname, "rb");
	if(!f){
		send_reply(_FNOP);
		return;
	}

	sockdesc s;
	if((s = req_cnct((const char *)remoteIP, (const char *)dataport)) < 0){
		send_reply(_GET);
		return;
	}
	if(!send_reply(sGET)) return;	//server starts to send data
	
	myftp_data md(f, s);
	if(md.send_file()) send_reply(GET);	//file successfully received by server
	else send_reply(_GET);
	fclose(f);
	close(s);		//very important, after s is closed, recv() returns 0 at client side, indicating completion of file transfer
}

//ls command
void server::els(){
	sockdesc s;
	if((s = req_cnct((const char *)remoteIP, (const char *)dataport)) < 0){	//server tries to connect to client on received dataport
		send_reply(_LS);
		return;
	}
	if(!send_reply(sLS)) return;	//server starts to send listing
	FILE *f = (FILE *)popen(command, "r");
	myftp_data md(f, s);	//myftp_data obj to handle file transfer
	if(md.send_file()) send_reply(LS);	//listing successful
	else send_reply(_LS);
	pclose(f);
	close(s);
}

//cd command
void server::ecd(){	//check eput() for comments
	int argc;
	char **argv;
	argc = pargs(command, &argv);
	if(argc < 0){	//invalid argument, check parse.cpp for details
		send_reply(_INVARG);
		return;
	}
	if(argc == 1){
		send_reply(_TFARG);
		return;
	}

	if(!chdir(argv[1])) send_reply(CD); //directory change successful
	else send_reply(_CD);
}

//pwd command
void server::epwd(){
	FILE *f = (FILE *)popen(command, "r");
	char line[MAXSIZE];
	fgets(line, sizeof(line), f);
	send_reply(line);
	pclose(f);
}

//port command
void server::eport(){
	char *p = command + 5;
	while((*p == ' ')||(*p == '\t')) p++;
	dataport = get_str((const char *)p); //get_str() in parse.cpp
	send_reply(PORT);	//port received
}

//child process starts from here to serve a client
void server::serve_client(){
	cout << "Connected to: " << remoteIP << endl;
	close(listener);	//child doesn't need the listener
	send_reply(CONNECTED);
	while(1){
		char buffer[MAXSIZE];
		stringstream ss;
		int n = recv(newsd, command, sizeof command, 0);
		if(n < 0) continue;
		if(n == 0) break;	//connection closed
		ss << command;
		ss >> buffer;
		if(!strcmp(buffer, "ls")) els();
		else if(!strcmp(buffer, "cd")) ecd();
		else if(!strcmp(buffer, "put")) eput();
		else if(!strcmp(buffer, "get")) eget();
		else if(!strcmp(buffer, "pwd")) epwd();
		else if(!strcmp(buffer, "port")) eport();
		else if(!strcmp(buffer, "quit")){
			if(send_reply(QUIT))
				break;
		}
		else send_reply(_INVALID);
	}
	cout << "Connection closed by: " << remoteIP << endl;
	close(newsd);
	exit(0);
}

//parent server process runs here
void server::serve(){
	sockdesc lsd;
	struct sigaction sa;
	if((lsd = get_listener_sock()) < 0){ //get a socket to listen for connections
		prerror(lsd);
		exit(1);
	}

	sa.sa_handler = sigchld_handler; //reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if(sigaction(SIGCHLD, &sa, NULL) == -1){
		perror("sigaction");
		exit(1);
	}

	while(1){ //main loop for accepting connections
		newsd = accept_cnct();
		if(newsd < 0){
			prerror(newsd);
			continue;
		}

		if(!fork()) serve_client(); //fork a child process to serve the new connection
		close(newsd); //parent doesn't need this
	}
}
