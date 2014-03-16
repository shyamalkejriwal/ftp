#ifndef SERVER_H
#define SERVER_H
#include "server.hpp"
#endif

#include "parse.cpp"

void sigchld_handler(int s){
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

server::server() : connection(){
	dataport = NULL;
}

server::server(int blog) : connection(blog){
	dataport = NULL;
}

server::server(const char *port, int blog) : connection(port,blog){
	dataport = NULL;
}

bool server::send_reply(const char *s){
	if(send(newsd, (const void *)s, strlen(s)+1, 0) == -1){
    	perror("Reply fails");
    	return false;
	}
	return true;
}

void server::eput(){
	int argc;
	char **argv, *fname;
	argc = pargs(command, &argv);
	if(argc == 1){
		send_reply(_TFARG);
		return;
	}
	else if(argc == 2) fname = argv[1];
	else fname = argv[2];
	if(access(fname, F_OK) != -1){
    	send_reply(_FXTS);
    	return;
	}

	FILE *f = fopen(fname, "wb");
	if(!f){
		send_reply(_FNOP);
		return;
	}

	sockdesc s;
	if((s = req_cnct((const char *)remoteIP, (const char *)dataport)) < 0){
		send_reply(_PUT);
		return;
	}
	if(!send_reply(rPUT)) return;
	
	myftp_data md(f, s);
	if(md.recv_file()) send_reply(PUT);
	else send_reply(_PUT);
	fclose(f);
	close(s);
}

void server::eget(){
	int argc;
	char **argv, *fname;
	argc = pargs(command, &argv);
	if(argc == 1){
		send_reply(_TFARG);
		return;
	}
	
	fname = argv[1];
	if(access(fname, F_OK) == -1){
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
	if(!send_reply(sGET)) return;
	
	myftp_data md(f, s);
	if(md.send_file()) send_reply(GET);
	else send_reply(_GET);
	fclose(f);
	close(s);		//very important
}

void server::els(){
	sockdesc s;
	if((s = req_cnct((const char *)remoteIP, (const char *)dataport)) < 0){
		send_reply(_LS);
		return;
	}
	if(!send_reply(sLS)) return;
	FILE *f = (FILE *)popen(command, "r");
	myftp_data md(f, s);
	if(md.send_file()) send_reply(LS);
	else send_reply(_LS);
	pclose(f);
	close(s);
}

void server::ecd(){
	int argc;
	char **argv;
	argc = pargs(command, &argv);
	if(argc < 0){
		send_reply(_INVARG);
		return;
	}
	if(argc == 1){
		send_reply(_TFARG);
		return;
	}

	if(!chdir(argv[1])) send_reply(CD);
	else send_reply(_CD);
}

void server::epwd(){
	FILE *f = (FILE *)popen(command, "r");
	char line[MAXSIZE];
	fgets(line, sizeof(line), f);
	send_reply(line);
	pclose(f);
}

void server::eport(){
	char *p = command + 5;
	while((*p == ' ')||(*p == '\t')) p++;
	dataport = get_str((const char *)p);
	send_reply(PORT);
}

void server::serve_client(){
	cout << "Connected to: " << remoteIP << endl;
	close(listener);	//child doesn't need the listener
	send_reply(CONNECTED);
	while(1){
		char buffer[MAXSIZE];
		stringstream ss;
		int n = recv(newsd, command, sizeof command, 0);
		if(n < 0) continue;
		if(n == 0) break;
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

void server::serve(){
	sockdesc lsd;
	struct sigaction sa;
	if((lsd = get_listener_sock()) < 0){
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

	while(1){
		newsd = accept_cnct();
		if(newsd < 0){
			prerror(newsd);
			continue;
		}

		if(!fork()) serve_client();
		close(newsd); //parent doesn't need this
	}
}
