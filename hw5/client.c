#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char* argv[]){
	int in_sockfd, w_size, res, flags;
	struct sockaddr_in ser_addr;
	char reading_buffer[1024];
	char writing_buffer[1024];
	char *name;
	char *ip = "127.0.0.1";
	char *port = "8080";
	fd_set r_fdset;
	fd_set w_fdset;
	w_size = sizeof(writing_buffer);
	if(argc == 3){
		name = argv[0];
		ip = argv[1];
		port = argv[2];
	}
	if(argc == 2){
		name = argv[0];
		ip = argv[1];
	}
	if(argc == 1){
		name = argv[0];
	}
	if((in_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("Failed to create in_sockfd\n");
		exit(EXIT_FAILURE);
	}

	memset(&ser_addr, '0', sizeof(ser_addr));
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_port = htons((int)strtol(port, (char **)NULL, 10));

	if((inet_pton(AF_INET, ip, &ser_addr.sin_addr)) <= 0){
		perror("Not a valid ip address\n");
		exit(EXIT_FAILURE);
	}

	if((connect(in_sockfd, (struct sockaddr *) &ser_addr, sizeof(ser_addr))) < 0){
		perror("Failed to connect to server\n");
		exit(EXIT_FAILURE);
	}

	flags = fcntl(in_sockfd, F_GETFL, 0);
	if(flags == -1){
        perror("Failed to get flags\n");
        exit(EXIT_FAILURE);
    }
    fcntl(in_sockfd, F_SETFL, flags | O_NONBLOCK);
	FD_ZERO(&r_fdset);
	FD_ZERO(&w_fdset);
	FD_SET(STDIN_FILENO, &r_fdset);
	FD_SET(in_sockfd, &w_fdset);
	FD_SET(in_sockfd, &r_fdset);

	while(1){
		if((res = select(in_sockfd+1, &r_fdset, &w_fdset, NULL, NULL)) < 0){
			perror("Failed to use select function\n");
			exit(EXIT_FAILURE);
		}
		if(res == 0)
			continue;
		if(FD_ISSET(STDIN_FILENO, &r_fdset)){
			read(STDIN_FILENO, writing_buffer, w_size);
		}
		if(FD_ISSET(in_sockfd, &r_fdset)){
//`			FD_CLR(in_sockfd, &r_fdset);
           	 if((read(in_sockfd, reading_buffer, 1024)) < 0){
         		perror("Failed to read server message\n");
                exit(EXIT_FAILURE);
            }
            printf("Server: %s", reading_buffer);
			memset(&reading_buffer, 0, sizeof(reading_buffer));
        }
		if(FD_ISSET(in_sockfd, &w_fdset)){
//			FD_CLR(in_sockfd, &w_fdset);
			send(in_sockfd, name, strlen(name), 0);
			send(in_sockfd, writing_buffer, sizeof(writing_buffer), 0);
			memset(&writing_buffer, 0, sizeof(writing_buffer));
		}
	}
}
