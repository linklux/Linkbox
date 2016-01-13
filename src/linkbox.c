// linkbox.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define PORT "3490" 
#define SERVER_ADDRESS "192.168.2.10"
#define CHUNK_SIZE 8196

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET)
		return &(((struct sockaddr_in*)sa)->sin_addr);

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

char* getFileBytes(const char *name, size_t *length) {
	FILE *fh = fopen(name, "r");

	fseek(fh, 0, SEEK_END);
	*length = ftell(fh);
	char *bytes = malloc(*length);

	fseek(fh, 0, SEEK_SET);
	fread(bytes, 1, *length, fh);
	fclose(fh);

	return bytes;
}

// Send all bytes in a file, split up if it's too large
int sendAll(int sfd, char *buffer, size_t length) {
	ssize_t n;
	const char *p = buffer;

	while(length > 0) {
		n = send(sfd, p, length, 0);

		if(n <= 0) break;

		p += n;
		length -= n;

		printf("Send bytes: %i (%i left) \n", n, length);
	}

	return n <= 0 ? -1 : 0;
}

// TODO Split up this massive function
int main(int argc, char *argv[]) {
	int sockfd, numbytes, rv;  
	struct addrinfo hints, *servinfo, *p;
	char s[INET6_ADDRSTRLEN];

	if (argc != 2) {
		fprintf(stderr,"File path not given, usage: linkbox <path/to/file> \n");
		exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(SERVER_ADDRESS, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
	printf("client: connecting to %s\n", s);

	// Cleanup the servinfo struct since we're done with it
	freeaddrinfo(servinfo);

	// TODO if a file is larger than a chunk (256 bytes), split it up
	size_t length;
	char *file_data = getFileBytes(argv[1], &length);

	send(sockfd, argv[1], strlen(argv[1]), 0);

	if(sendAll(sockfd, file_data, length) == -1) {
		perror("Send all");
		exit(1);
	}

	send(sockfd, "EOF", 3, 0);

	//if((send(sockfd, file_data, strlen(file_data), 0) == -1)) {
	//	perror("send");
	//	exit(1);
	//}

	printf("Transmition successful, total bytes send: %i \n", length);

	close(sockfd);

	return 0;
}

