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

// TODO read these from a config file
#define SERVER_ADDRESS "192.168.2.10"
#define CHUNK_SIZE 256

// TODO include a client ID for authentication

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET)
		return &(((struct sockaddr_in*)sa)->sin_addr);

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

char* getFileBytes(FILE *fp, size_t amount, size_t *total_read) {
	char *bytes = malloc(amount);
	*total_read = fread(bytes, 1, amount, fp);

	return bytes;
}

int getFileSize(FILE *fp) {
	fseek(fp, 0, SEEK_END);
	size_t length = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	return length;
}

int sendAll(int sfd, char *buffer, size_t length) {
	ssize_t n;
	const char *p = buffer;

	for(;;) {
		n = send(sfd, p, length, 0);

		if(n <= 0) break;

		p += n;
		length -= n;

		printf("Send bytes: %i (%i left) \n", n, length);

		if(length <= 0) break;
	}

	return n <= 0 ? -1 : 0;
}

/* Send the header information required by the server to proces the stream. This only includes 
 * the and desired file name for now */
int sendHeader(int sfd, char *filename) {
	size_t header_size = strlen(filename) + 4;

	char header[header_size];
	strcpy(header, filename);

	uint32_t header_size_network = htonl(header_size);

	if(send(sfd, &header_size_network, sizeof(header_size_network), 0) == -1) {
		perror("send header size");
		exit(1);
	}

	printf("Send header size to server (%i bytes) \n", header_size);

	if(send(sfd, header, header_size, 0) == -1) {
		perror("send header data");
		exit(1);
	}

	printf("Send header data to server: %s \n", header);

	return sizeof(header_size_network) + header_size;
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

	// Send header required by the server
	size_t header_bytes = sendHeader(sockfd, argv[1]);

	FILE *fp;
	fp = fopen(argv[1], "r");

	size_t file_size = getFileSize(fp);
	size_t bytes_left = file_size;
	size_t total_read;

	while(bytes_left > 0) {
		char *file_data = getFileBytes(fp, CHUNK_SIZE * 1000, &total_read);

		if(sendAll(sockfd, file_data, total_read) == -1) {
			perror("Send all");
			exit(1);
		}

		file_data += total_read;
		bytes_left -= total_read;
	}

	// TODO receive and print server response, a simple progress bar with large files (pacman style?)

	printf("Transmission successful, total bytes send: %i \n", header_bytes + file_size);

	close(sockfd);

	return 0;
}

