#ifndef CONNECT_H
#define CONNECT_H
#include "connect.hpp"
#endif

//create a copy of string s
char *cstr(const char *s){
	char *s1 = new char[strlen(s)+1];
	strcpy(s1,s);
	return s1;
}

connection::connection(){
	lport = NULL;
	backlog = BACKLOG;
	listener = -1;
	remoteIP = new char[INET6_ADDRSTRLEN];
}

connection::connection(int blog){
	lport = NULL;
	backlog = blog;
	listener = -1;
	remoteIP = new char[INET6_ADDRSTRLEN];
}

connection::connection(const char *port, int blog){
	lport = cstr(port);
	backlog = blog;
	listener = -1;
	remoteIP = new char[INET6_ADDRSTRLEN];
}

//set listener port
void connection::set_lport(const char *port){
	lport = cstr(port);
}

//get sockaddr, IPv4 or IPv6:
void *connection::get_in_addr(struct sockaddr *sa){
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

//get listener socket descriptor
sockdesc connection::get_listener_sock(){
	struct addrinfo hints, *ai, *p;
	int yes = 1;
	memset(&hints,0,sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	
	if((ecode = getaddrinfo(NULL, lport, &hints, &ai)) != 0){
		return (listener = -2);
	}
	
	//loop through all the results and bind to the first we can
	for(p = ai; p != NULL; p = p->ai_next){
		listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
	    if(listener < 0){
		    continue;
	    }
		
		//lose the pesky "address already in use" error message
		setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

		if(bind(listener, p->ai_addr, p->ai_addrlen) < 0){
			close(listener);
			continue;
		}
		break;
	}

	//if we got here, it means we didn't get bound
	if(p == NULL){
		return (listener = -3);
	}

	freeaddrinfo(ai); //all done with this

    // listen
    if(listen(listener, backlog) == -1){
      	return (listener = -4);
   	}
	return listener;
}

//accept a pending request for connection by a peer
sockdesc connection::accept_cnct(){
	
	struct sockaddr_storage remoteaddr; //client address
    socklen_t addrlen;
	addrlen = sizeof remoteaddr;
	newsd = accept(listener,(struct sockaddr *)&remoteaddr,&addrlen);

    if(newsd != -1){
		inet_ntop(remoteaddr.ss_family, 
					get_in_addr((struct sockaddr*)&remoteaddr), remoteIP, INET6_ADDRSTRLEN);
	}
	return newsd;
}

//request connection on a remote peer or server
sockdesc connection::req_cnct(const char *ip, const char *port){
	sockdesc sockfd;
	struct addrinfo hints, *servinfo, *p;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if((ecode = getaddrinfo(ip, port, &hints, &servinfo)) != 0){
		return -2;
	}

	//loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next){
		if((sockfd = socket(p->ai_family, p->ai_socktype,
			p->ai_protocol)) == -1){
			continue;
		}

		if(connect(sockfd, p->ai_addr, p->ai_addrlen) == -1){
			close(sockfd);
			continue;
		}
		break;
	}

	if(p == NULL){
		return -5;
	}

	freeaddrinfo(servinfo); //all done with this structure
	return sockfd;
}

//print most recent error
void connection::prerror(int code){
	switch(code){
		case -1:
			perror("accept");
			break;
		case -2:
			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ecode)); //uses ecode member
			break;
		case -3:
			fprintf(stderr, "bind: failed to bind\n");
			break;
		case -4:
			perror("listen");
			break;
		case -5:
			fprintf(stderr, "error: failed to connect\n");
			break;
	}
	return;
}

char *connection::get_remoteIP(){
	return remoteIP;
}

myftp_data::myftp_data(FILE *f, sockdesc s){
	fp = f;
	sd = s;
	nbytes = 0;
}

//send file
bool myftp_data::send_file(){
	char buffer[MAXSIZE];
	size_t count;
	nbytes = 0;
	while(!feof(fp)){	//end of file
    	count = fread((void *)buffer, 1, sizeof(buffer), fp);	//read next data chunk from file
    	if(ferror(fp) || (send(sd, (const void *)buffer, count, 0) == -1)){	//send data chunk on sd
        	return false;
    	}
    	nbytes += count;
	}
	return true;
}

//receive file
bool myftp_data::recv_file(){
	char buffer[MAXSIZE];
	int count;
	nbytes = 0;
	while((count = recv(sd, (void *)buffer, sizeof(buffer), 0))){ //recv data chunk on sd
		if(count > 0) nbytes += count;
		if((count < 0)||(fwrite((const void *)buffer, 1, count, fp) < (unsigned int)count)) //write data chunk to file
			return false;
	}
	return true;
}

//get size of last transfer
size_t myftp_data::getsize(){
	return nbytes;
}