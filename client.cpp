#ifndef CLIENT_H
#define CLIENT_H
#include "client.hpp"
#endif

#include "parse.cpp"

#define FAIL "local: command fails\n"

client::client(const char *IP, const char *port) : connection(){
	server_IP = get_str(IP);	//get_str() in parse.cpp
	server_port = get_str(port);
	ctrlsd = -1;
}

//receive a reply from server
bool client::get_reply(){
	char s[MAXSIZE];
	int n;
	if((n = recv(ctrlsd, (void *)s, sizeof s, 0)) < 0) return false;
	else if(n == 0){	//Connection closed
		cout << "Connection closed by server \n";
		close(ctrlsd);
		exit(1);
	}
	cout << s;
	return true;
}

//receive a reply and match if intended
bool client::get_reply(int code){
	char s[MAXSIZE];
	int n;
	if((n = recv(ctrlsd, (void *)s, sizeof s, 0)) < 0) return false;
	else if(n == 0){	//Connection closed
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

//send a listener port to server for data transfer
bool client::eport(){
	int count  = 0;
	bool flag = false;
	char buffer[MAXSIZE] = "port ";
	char lp[10];
	while(count < MAXTRY){ //try MAXTRY number of listener ports, start with CPORT
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
	if(send(ctrlsd, (const void *)buffer, strlen(buffer)+1, 0) == -1){ //send port
		return false;
	}
	if(!get_reply(cPORT)) return false; //check if port received by server
	return true;
}

//ls on server
void client::els(){
	if(!eport()){	//send a listener port to server for data transfer 
		cout << FAIL;
		return;
	}

	if(send(ctrlsd, (const void *)command, strlen(command)+1, 0) == -1){ //send command to server
		close(listener);
		cout << FAIL;
		return;
	}
	
	if(!get_reply(csLS)){	//check if server connected on port sent and starts listing
		close(listener);
		cout << FAIL;
		return;
	}

	sockdesc s;
	s = accept_cnct();	//accept connection from server
	myftp_data md(stdout, s); //myftp_data obj to handle file transfer, stdout gets listing
	md.recv_file();
	get_reply();	//success reply from server
	close(s);
	close(listener);
}

//cd on server
void client::ecd(){
	if(send(ctrlsd, (const void *)command, strlen(command)+1, 0) == -1){ //send command to server
		cout << FAIL;
	}
	else get_reply();
}

void client::eput(){
	int argc;	//number of arguments
	char **argv, *fname;	//argv is array of arguments, fname is filename
	argc = pargs(command, &argv);	//parse command to get arguments
	if(argc < 0) return;	//invalid argument
	if(argc == 1){	//the only arg is put, fname required
		cout << "local: File name not specified\n";
		return;
	}
	fname = argv[1];	//file to be transferred
	if(access(fname, F_OK) == -1){	//file doesn't exist
    	cout << "local: File doesn't exist\n";
    	return;
	}

	FILE *f = fopen(fname, "rb");
	if(!f){	//file couldn't be opened
		cout << "local: File couldn't be opened\n";
		return;
	}

	if(!eport()){	//send a listener port to server for data transfer
		cout << FAIL;
		fclose(f);
		return;
	}
	if(send(ctrlsd, (const void *)command, strlen(command)+1, 0) == -1){	//send command to server
		cout << FAIL;
		close(listener);
		fclose(f);
		return;
	}
	if(!get_reply(crPUT)){	//check if server ready to receive data
		cout << FAIL;
		close(listener);
		fclose(f);
		return;
	}
	
	sockdesc s;
	s = accept_cnct();	//accept connection from server
	myftp_data md(f, s);	//myftp_data obj to handle file transfer
	md.send_file();
	close(s);	//very important, after s is closed, recv() returns 0 at server side, indicating completion of file transfer
	get_reply();	//success reply from server
	fclose(f);
	close(listener);
}

void client::eget(){ //check eput() for comments
	int argc;
	char **argv, *fname;
	argc = pargs(command, &argv);
	if(argc < 0) return;
	if(argc == 1){
		cout << "local: File name not specified\n";
		return;
	}
	else if(argc == 2) fname = argv[1];	//user inputs only remote fname, remote fname becomes local fname
	else fname = argv[2];	//user inputs local fname
	if(access(fname, F_OK) != -1){	//file with fname already exists, can't be overwritten
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
		remove(fname);	//newly created file(fname) removed
		fclose(f);
		return;
	}
	if(send(ctrlsd, (const void *)command, strlen(command)+1, 0) == -1){
		cout << FAIL;
		remove(fname);	//newly created file(fname) removed
		close(listener);
		fclose(f);
		return;
	}
	if(!get_reply(csGET)){ //check if server successfully starts sending data
		cout << FAIL;
		remove(fname);	//newly created file(fname) removed
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

//pwd on server
void client::epwd(){
	if(send(ctrlsd, (const void *)command, strlen(command)+1, 0) == -1){
		cout << FAIL;
	}
	else get_reply();
}

//termianl command on local host
void client::elcmd(){
	FILE *f = (FILE *)popen(command+1, "r");
	char buffer[MAXSIZE];
	while(fgets(buffer, sizeof(buffer), f)){
  		printf("%s", buffer);
	}
	pclose(f);
}

//cd on local host, check eput() for comments
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

//quit session
void client::equit(){
	if(send(ctrlsd, (const void *)command, strlen(command)+1, 0) == -1){ //send command
		cout << FAIL;
		return;
	}
	if(get_reply(cQUIT)){ //check if QUIT is successful
		close(ctrlsd);
		exit(0);
	}
}

//connect, input commands and execute
void client::get_service(){
	
	if((ctrlsd = req_cnct(server_IP, server_port)) < 0){	//request for connection on server
		prerror(ctrlsd);
		exit(1);
	}

	if(!get_reply(cCONNECTED)){	//check if connection to server is successful
		cout << "Connection couldn't be established\n";
		exit(1);
	}

	while(1){	//input commands and execute
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


