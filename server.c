#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include <stdio.h>
#include <stdbool.h>

#ifndef PORT
#define PORT "3490"
#endif
#define QUEUE_SIZE 10 // max amout of users in queue

// TODO: replace the awful snake case with glorious camel case (and do the static inline .h thing)
// TODO: add error checking (and possibly logging) to the calls
// TODO: use freeaddrinfo
// TODO: implement send() and recv() properly

int main() {
	struct sockaddr_storage clientaddr;
	socklen_t addrsize;
	struct addrinfo hints, *res;
	int sockfd, newfd;
	int status;
	const int trueFlag = 1;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC; // AF_INET for IPv4, AF_INET6 for IPv6, AF_UNSPEC for either one
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((status = getaddrinfo(NULL, PORT, &hints, &res)) != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		exit(1);
	}

	if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
		int errsv = errno;
		fprintf(stderr, "socket() failed, errno %d\n", errsv);
		exit(1);
	}

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &trueFlag, sizeof(trueFlag)) == -1) {
		int errsv = errno;
		fprintf(stderr, "setsockopt() failed, errno %d\n", errsv);
		exit(1);
	}

	if (bind(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
		int errsv = errno;
		fprintf(stderr, "bind() failed, errno %d\n", errsv);
		exit(1);
	}

	if (listen(sockfd, QUEUE_SIZE) == -1) {
		int errsv = errno;
		fprintf(stderr, "listen() failed, errno %d\n", errsv);
		exit(1);
	}
	
	fprintf(stderr, "Server up and running\n"); // maybe add gethostname() here

	addrsize = sizeof(clientaddr);
	while (1) { // swap this out for a proper connection pool manager (and preferrably something multithreaded)
		struct sockaddr peeraddr;
		int addrlen = sizeof(struct sockaddr);
		char peerip[INET6_ADDRSTRLEN];

		memset(&peeraddr, 0, addrlen);
		newfd = accept(sockfd, (struct sockaddr*)&clientaddr, &addrsize); // this is the shit you would multithread
		if (getpeername(newfd, &peeraddr, &addrlen) == -1) { // figure out why this is failing
			int errsv = errno;
			fprintf(stderr, "getpeername() failed, errno %d\n", errsv);
		}
		// inet_ntop(peeraddr.sa_family, peeraddr.sa_data, peerip, INET6_ADDRSTRLEN); // figure out a nice way of error handling this function because this function is really stupid
		if (inet_ntop(peeraddr.sa_family, peeraddr.sa_data, peerip, INET6_ADDRSTRLEN) == NULL) {
			int errsv = errno;
			fprintf(stderr, "inet_ntop() failed, errno %d\n", errsv);
		}
		fprintf(stderr, "new connection from %s\n", peerip);


		// char* msg = "Hello there, General Kenobi\n";
		char* msg = "Halló þarna, Kenöbi hershöfðingi\n";
		int len, bytesSent;
		len = strlen(msg);
		bytesSent = send(newfd, msg, len, 0); // do error handling for this
		if (close(newfd) == -1) {
			int errsv = errno;
			fprintf(stderr, "close() on %d failed, errno %d\n", errsv, errsv);
		}
	}
}
