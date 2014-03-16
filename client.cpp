#ifndef CLIENT_H
#define CLIENT_H
#include "client.hpp"
#endif

#include "parse.cpp"

#define FAIL "local: command fails\n"

client::client(const char *IP, const char *port) : connection(){
	server_IP = get_str(IP);
	server_port = get_str(port);
	ctrlsd = -1;
}

bool client::get_reply(){
	char s[MAXSIZE];
	int n;
	if((n = recv(ctrlsd, (void *)s, sizeof s, 0)) < 0) return false;
	else if(n == 0){
		cout << "Connection closed by server \n";
		close(ctrlsd);
		exit(1);
	}
	cout << s;
	return true;
}

bool client::get_reply(int code){
	char s[MAXSIZE];
	int n;
	if((n = recv(ctrlsd, (void *)s, sizeof s, 0)) < 0) return false;
	else if(n == 0){
		cout << "Connection closed by server \n";
		close(ctrlsd);
		exit(1);
	}
	cout << s;
	stringstream ss;
	ss << s;
	ss >> n;
	return (n == code);	
}

bool client::eport(){
	int count  = 0;
	bool flag = false;
	char buffer[MAXSIZE] = "port ";
	char lp[10];
	while(count < MAXTRY){
		int p = atoi(CPORT) + count;
		stringstream ss;
		ss << p;
		ss >> lp;
		set_lport(lp);
		if(get_listener_sock() > 0){
			flag = true;
			break;
		}
		count++;
	}
	if(!flag) return false;
	strcat(buffer, lp);
	if(send(ctrlsd, (const void *)buffer, strlen(buffer)+1, 0) == -1){
		return false;
	}
	if(!get_reply(cPORT)) return false;
	return true;
}

void client::els(){
	if(!eport()){
		cout << FAIL;
		return;
	}

	if(send(ctrlsd, (const void *)command, strlen(command)+1, 0) == -1){
		close(listener);
		cout << FAIL;
		return;
	}
	
	if(!get_reply(csLS)){
		close(listener);
		cout << FAIL;
		return;
	}

	sockdesc s;
	s = accept_cnct();
	myftp_data md(stdout, s);
	md.recv_file(); 
	get_reply();
	close(s);
	close(listener);
}

void client::ecd(){
	if(send(ctrlsd, (const void *)command, strlen(command)+1, 0) == -1){
		cout << FAIL;
	}
	else get_reply();
}

void client::eput(){
	int argc;
	char **argv, *fname;
	argc = pargs(command, &argv);
	if(argc < 0) return;
	if(argc == 1){
		cout << "local: File name not specified\n";
		return;
	}
	fname = argv[1];
	if(access(fname, F_OK) == -1){
    	cout << "local: File doesn't exist\n";
    	return;
	}

	FILE *f = fopen(fname, "rb");
	if(!f){
		cout << "local: File couldn't be opened\n";
		return;
	}

	if(!eport()){
		cout << FAIL;
		fclose(f);
		return;
	}
	if(send(ctrlsd, (const void *)command, strlen(command)+1, 0) == -1){
		cout << FAIL;
		close(listener);
		fclose(f);
		return;
	}
	if(!get_reply(crPUT)){
		cout << FAIL;
		close(listener);
		fclose(f);
		return;
	}
	
	sockdesc s;
	s = accept_cnct();
	myftp_data md(f, s);
	md.send_file();
	close(s);		//very important
	get_reply();
	fclose(f);
	close(listener);
}

void client::eget(){
	int argc;
	char **argv, *fname;
	argc = pargs(command, &argv);
	if(argc < 0) return;
	if(argc == 1){
		cout << "local: File name not specified\n";
		return;
	}
	else if(argc == 2) fname = argv[1];
	else fname = argv[2];
	if(access(fname, F_OK) != -1){
    	cout << "local: File already exists\n";
    	return;
	}

	FILE *f = fopen(fname, "wb");
	if(!f){
		cout << "local: File couldn't be opened\n";
		return;
	}
	if(!eport()){
		cout << FAIL;
		fclose(f);
		return;
	}
	if(send(ctrlsd, (const void *)command, strlen(command)+1, 0) == -1){
		cout << FAIL;
		close(listener);
		fclose(f);
		return;
	}
	if(!get_reply(csGET)){
		cout << FAIL;
		close(listener);
		fclose(f);
		return;
	}
	
	sockdesc s;
	s = accept_cnct();
	myftp_data md(f, s);
	md.recv_file(); 
	get_reply();
	fclose(f);
	close(s);
	close(listener);
}

void client::epwd(){
	if(send(ctrlsd, (const void *)command, strlen(command)+1, 0) == -1){
		cout << FAIL;
	}
	else get_reply();
}

void client::elcmd(){
	FILE *f = (FILE *)popen(command+1, "r");
	char buffer[MAXSIZE];
	while(fgets(buffer, sizeof(buffer), f)){
  		printf("%s", buffer);
	}
	pclose(f);
}

void client::elcd(){
	int argc;
	char **argv;
	argc = pargs(command, &argv);
	if(argc < 0) return;
	if(argc == 1){
		cout << "local: Too few arguments\n";
		return;
	}

	if(!chdir(argv[1])) cout << "local: Directory change successful\n";
	else cout << "local: Directory couldn't be changed\n";
}

void client::equit(){
	if(send(ctrlsd, (const void *)command, strlen(command)+1, 0) == -1){
		cout << FAIL;
		return;
	}
	if(get_reply(cQUIT)){
		close(ctrlsd);
		exit(0);
	}
}

void client::get_service(){
	int n;
	if((ctrlsd = req_cnct(server_IP, server_port)) < 0){
		prerror(ctrlsd);
		exit(1);
	}

	if(!get_reply(cCONNECTED)){
		cout << "Connection couldn't be established\n";
		exit(1);
	}

	while(1){
		char buffer[MAXSIZE];
		stringstream ss;
		strcpy(command, "");
		cout << "ftp> ";
		scanf("%[^\n]", command);
		getchar();
		if(!strcmp(command, "")) continue;
		ss << command;
		ss >> buffer;
		if(!strcmp(buffer, "ls")) els();
		else if(!strcmp(buffer, "cd")) ecd();
		else if(!strcmp(buffer, "put")) eput();
		else if(!strcmp(buffer, "get")) eget();
		else if(!strcmp(buffer, "pwd")) epwd();
		else if(!strcmp(buffer, "!ls")) elcmd();
		else if(!strcmp(buffer, "!cd")) elcd();
		else if(!strcmp(buffer, "!pwd")) elcmd();
		else if(!strcmp(buffer, "quit")) equit();
		else cout << "Invalid command\n";
	}
}


