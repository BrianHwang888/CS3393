#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>


int main(int argc, char* argv[]){
	int in_sockfd;
	int conn_sockfd;
	int opt = 1;
	int res, flags, flag;
	int userid = 0;
	int addr_size, w_size, o_size;
	char reading_buf[1024];
	char writing_buf[1024];	
	char* name;
	char* port = "8080";
	char* open_mess = "Connected";
	struct sockaddr_in addr;
	fd_set r_fdset;
	fd_set w_fdset;
	if(argc == 2){
		name = argv[0];
		port = argv[1];
    }
    if(argc == 1){
		name = argv[0];
    }

	addr.sin_family = AF_INET;
	addr.sin_port = htons((int)strtol(port, (char **)NULL, 10));
	addr.sin_addr.s_addr = INADDR_ANY;
	addr_size = sizeof(addr);
	w_size = sizeof(writing_buf);
	o_size = strlen(open_mess);

	if((in_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		fprintf(stderr, "Failed to setup socket %d: %s\n", in_sockfd,strerror(errno));
		exit(EXIT_FAILURE);
	}
	if((setsockopt(in_sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) < 0){
		perror("Failed to set socket options\n");
		exit(EXIT_FAILURE);
	}
	if((bind(in_sockfd, (struct sockaddr *) &addr, (socklen_t) addr_size)) < 0){
		perror("Failed to bind to socket in\n");
		exit(EXIT_FAILURE);
	}
	if((listen(in_sockfd, 0)) < 0){
		perror("Failed to listen to socket in\n");
		exit(EXIT_FAILURE);
	}
	if((conn_sockfd = accept(in_sockfd, (struct sockaddr *) &addr, (socklen_t *) &addr_size)) < 0){
		perror("Failed to accept connection\n");
		exit(EXIT_FAILURE);
	}
	flags = fcntl(in_sockfd, F_GETFL, 0);
	if(flags == -1){
		perror("Failed to get flags\n");
		exit(EXIT_FAILURE);
	}
	flags = fcntl(in_sockfd, F_GETFL, 0);
    if(flags == -1){
        perror("Failed to get flags\n");
        exit(EXIT_FAILURE);
    }
	flag = fcntl(conn_sockfd, F_GETFL, 0);
    if(flag == -1){
        perror("Failed to get flags\n");
        exit(EXIT_FAILURE);
    }
	fcntl(in_sockfd, F_SETFL, flags | O_NONBLOCK);
	fcntl(conn_sockfd, F_SETFL, flag | O_NONBLOCK);
	send(conn_sockfd, open_mess, o_size, 0);



	FD_ZERO(&r_fdset);
    FD_ZERO(&w_fdset);
	FD_SET(STDIN_FILENO, &r_fdset);
	FD_SET(conn_sockfd, &r_fdset);
	FD_SET(conn_sockfd, &w_fdset);

	while(1){
		if((res = select(conn_sockfd+1, &r_fdset, &w_fdset, NULL, NULL)) < 0){
            perror("Failed to use select function\n");
            exit(EXIT_FAILURE);
        }
		if(res == 0)
			continue;
		if(FD_ISSET(STDIN_FILENO, &r_fdset)){
			read(STDIN_FILENO, writing_buf, w_size);
		}
		if(FD_ISSET(conn_sockfd, &r_fdset)){
//			FD_CLR(conn_sockfd, &r_fdset);
			if((read(conn_sockfd, reading_buf, 1024)) < 0){
				perror("Failed to read message\n");
				exit(EXIT_FAILURE);
			}
			printf("User %d: %s\n", userid, reading_buf);
			memset(&reading_buf, 0, sizeof(reading_buf));
		}
		if(FD_ISSET(conn_sockfd, &w_fdset)){
//			FD_CLR(conn_sockfd, &w_fdset);
			send(in_sockfd, name, strlen(name), 0);
			send(conn_sockfd, writing_buf, w_size, 0);
			memset(&writing_buf, 0, sizeof(writing_buf));
		}
	}
}

