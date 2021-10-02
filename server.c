#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
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
	struct sockaddr_storage client_addr;
	socklen_t addr_size;
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

	addr_size = sizeof(client_addr);
	while (1) { // swap this out for a proper connection pool manager (and preferrably something multithreaded)
		newfd = accept(sockfd, (struct sockaddr*)&client_addr, &addr_size); // this is the shit you would multithread
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
