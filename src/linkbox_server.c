// linkbox_server.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "3490"

#define BACKLOG 10
#define CHUNK_SIZE 4096

void sigchld_handler(int s) {
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}

void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) 
		return &(((struct sockaddr_in*)sa)->sin_addr);

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void writeFile() {

}

// TODO split up this massive function
int main(void) {
	int sockfd, new_fd, numbytes, rv, yes = 1; 
	char s[INET6_ADDRSTRLEN], chunk_data[CHUNK_SIZE];
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr;
	struct sigaction sa;
	socklen_t sin_size;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	// Use our local IP address for now, remove this line when working on external server
	hints.ai_flags = AI_PASSIVE;

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	// Cleanup the servinfo structince we're done with it
	freeaddrinfo(servinfo);

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;

	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("server: waiting for connections...\n");

	while(1) {  // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);

		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
		printf("server: got connection from %s\n", s);

		if (!fork()) { // this is the child process
			close(sockfd); // child doesn't need the listener

			// TODO first 'packet' should be a fixed size header, containing information about total size and field
			// position data. Create the file here and open it for writing 
			if((numbytes = recv(new_fd, chunk_data, CHUNK_SIZE, 0)) == -1) {
				perror("recv");
				exit(1);
			}

			printf("First chunk recieved from client: (%i bytes) \n", numbytes);

			// If the amount of bytes we got are equal the the chunk size, we have stuff qeued, get this stream by
			// calling recv() again. Repeat this until the received aren't equal to the chunk size anymore, meaning
			// the stream is finished
			while(numbytes == CHUNK_SIZE) {
				char new_chunk_data[CHUNK_SIZE];

				if((numbytes = recv(new_fd, new_chunk_data, CHUNK_SIZE, 0)) == -1) {
					perror("recv sub");
					exit(1);
				}

				// TODO write the received bytes to the openend file. If numbytes != CHUNK_SIZE (end of stream) close the file
				// and return the url (and later on, the password if selected) to the client
				printf("Subdata chunk received from client: (%i bytes) \n", numbytes);
			}

			//chunk_data[numbytes] = '\0';

			close(new_fd);
			break;
		}

		close(new_fd);  // parent doesn't need this
	}

	return 0;
}
