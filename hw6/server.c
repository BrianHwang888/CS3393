#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_BUFF 1024
#define MAX_NAMELEN 20
#define MAX_MESS 50
#define MAX_CLIENTS 5

typedef struct {
	int own_sock;
	char* name;

}client_node;


void* create_connection(void* thread);
void* read_message_queue();
void add_message_queue(char* message);
void add_client(client_node* client);
void remove_client(client_node* client);
void send_online_info(int sockfd);
void announce_joining(char* message, char* username);
void announce_leaving(char* message, char* username);
char* create_quit_mess(char* username, int is_window);

char* message_queue[MAX_MESS];
client_node* client_list[MAX_CLIENTS];

pthread_cond_t message_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t message_empty = PTHREAD_COND_INITIALIZER;
pthread_mutex_t message_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t client_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t connection_lock = PTHREAD_MUTEX_INITIALIZER;

/*
num_clients: holds the number of clients connected
front: points to the front of the message queue (the first message)
end: points to the index for the next incoming message
*/
int num_clients = 0, front = 0, end = 0;

//in_sock is server socket and conn_sock is the client socket
int main(void){
	int in_sock, conn_sock, addr_size;
	int port = 8080;
	int opt = 1;
	int* conn_sock_p;
	struct sockaddr_in addr;
	pthread_t tid, mess_tid;

	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;
	addr_size = sizeof(addr);

	if((in_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("Failed to setup socket\n");
		exit(EXIT_FAILURE);
	}

	if((setsockopt(in_sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) < 0 ){
		perror("Failed to set socket options\n");
		exit(EXIT_FAILURE);
	}

	if((bind(in_sock, (struct sockaddr *) &addr, (socklen_t) addr_size)) < 0){
        perror("Failed to bind to socket in\n");
        exit(EXIT_FAILURE);
    }

    if((listen(in_sock, 0)) < 0){
        perror("Failed to listen to socket in\n");
        exit(EXIT_FAILURE);
    }

	if(pthread_create(&mess_tid, NULL, read_message_queue, NULL) < 0){
		perror("Failed to create thread for reading message queue\n");
		exit(EXIT_FAILURE);
	}

	if(pthread_detach(mess_tid) < 0){
		perror("Failed to detach read_message_queue thread\n");
		exit(EXIT_FAILURE);
	}

	//Loop to spawn client threads
	while(1){
		if((conn_sock = accept(in_sock, (struct sockaddr *) &addr, (socklen_t *) &addr_size)) < 0){
			perror("Failed to accept connection\n");
			exit(EXIT_FAILURE);
		}
//		printf("Conn_sock: %d\n", conn_sock);
		conn_sock_p = malloc(sizeof(int));
		*conn_sock_p = conn_sock;
		if(num_clients == MAX_CLIENTS)
			close(conn_sock);

		if(num_clients < MAX_CLIENTS){
			if(pthread_create(&tid, NULL, create_connection, conn_sock_p) < 0 ){
				perror("Failed to create thread\n");
				exit(EXIT_FAILURE);
			}
			if(pthread_detach(tid) < 0){
				perror("Failed to detach thread\n");
				exit(EXIT_FAILURE);
			}
		}
	}

	close(in_sock);
	pthread_exit(0);
}

//the client thread
void* create_connection(void *sock){
	int* client_sock = sock;
	client_node* client = (client_node*)malloc(sizeof(client_node));
	char* username = malloc(MAX_NAMELEN * sizeof(char));
	char* welcome_message = "======================Welcome to the server===================== \nPlease enter your username:";
	char* message = malloc(sizeof(char) * MAX_BUFF);
	char* quit_mess_window, *quit_mess_other;

	if(write(*client_sock, welcome_message, strlen(welcome_message)) < 0){
		perror("Failed to send greetings\n");
		puts(strerror(errno));
		exit(EXIT_FAILURE);
	}

	memset(client, 0, sizeof(client_node));
	if(read(*client_sock, username, MAX_NAMELEN) < 0){
		perror("Failed to read username from client\n");
		exit(EXIT_FAILURE);
	}

	username[strlen(username)-1] = '\0';
	client->name = username;
	client->own_sock = *client_sock;
	add_client(client);
	send_online_info(*client_sock);

	announce_joining(message, client->name);

	quit_mess_window = create_quit_mess(username, 1);
	quit_mess_other = create_quit_mess(username, 0);

	while(1){
		if((read(client->own_sock, message, MAX_BUFF)) < 0)
			perror("Failed to read client message\n");

		puts(message);
		if(strcmp(message, quit_mess_window) == 0 || strcmp(message, quit_mess_other) == 0){
			puts("quit");
			announce_leaving(message, client->name);
			break;
		}

		printf("Message %sUsing socket: %d\n", message, client->own_sock);
		add_message_queue(message);
	}

	close(client->own_sock);
	remove_client(client);

	pthread_mutex_lock(&client_lock);
	num_clients--;
	pthread_mutex_unlock(&client_lock);

	free(client);
	free(message);
	free(username);
	free(quit_mess_window);
	free(quit_mess_other);

	pthread_exit(0);
}

//Reads a message from queue
void* read_message_queue(){
	while(1){
		pthread_mutex_lock(&message_lock);
		while(message_queue[front] == NULL)
			pthread_cond_wait(&message_full, &message_lock);

		pthread_mutex_lock(&client_lock);
		for(int m = 0; message_queue[m] != NULL && m < MAX_MESS; m++){
			for(int i = 0; client_list[i] != NULL && i < MAX_CLIENTS; i++){
				if(write(client_list[i]->own_sock, message_queue[m], strlen(message_queue[m])) < 0){
            		fprintf(stderr, "Failed to send message to socket: %d\n", client_list[i]->own_sock);
            		exit(EXIT_FAILURE);
        		}
			}

			memset(message_queue[m], 0, MAX_BUFF);
			message_queue[m] = NULL;
		}

		end = 0;
		pthread_cond_signal(&message_empty);
		pthread_mutex_unlock(&message_lock);
		pthread_mutex_unlock(&client_lock);
	}
	pthread_exit(0);
}

//adds messages to message queue
void add_message_queue(char* message){
   	pthread_mutex_lock(&message_lock);
	while(message_queue[end] != NULL)
		pthread_cond_wait(&message_empty, &message_lock);
	message_queue[end] = message;
	end++;
	if(end == MAX_BUFF)
		end = 0;
	pthread_cond_signal(&message_full);
    pthread_mutex_unlock(&message_lock);
}

//adds client to client list
void add_client(client_node* client){
	pthread_mutex_lock(&client_lock);
	for(int i = 0; i < MAX_CLIENTS; i++){
		if(client_list[i] == NULL){
			client_list[i] = client;
			num_clients++;
			break;
		}
	}
	pthread_mutex_unlock(&client_lock);
	printf("User: %s connected\n", client->name);
}

//removes a client from list
void remove_client(client_node* client){
	pthread_mutex_lock(&client_lock);
	for(int i = 0; i < MAX_CLIENTS; i++){
		if(client_list[i] == client){
			client_list[i] = NULL;
			break;
		}
	}
	pthread_mutex_unlock(&client_lock);
	printf("User: %s disconnected\n", client->name);
}

//sends online information to clients
void send_online_info(int sockfd){
	char *online_list = malloc(sizeof(char) * MAX_BUFF);

	strcat(online_list,"\n=================================================================================\nUsers online: \n");
	for(int i = 0; client_list[i] != NULL && i < MAX_CLIENTS; i++){
		strcat(online_list, client_list[i]->name);
		strcat(online_list, "\n");
	}
	online_list[strlen(online_list)] = '\0';
	pthread_mutex_lock(&client_lock);
    if(write(sockfd, online_list, strlen(online_list)) < 0){
        fprintf(stderr, "Failed to send online clients to socket: %d\n", sockfd);
        exit(EXIT_FAILURE);
    }
    pthread_mutex_unlock(&client_lock);
	free(online_list);
}


/*
Generates the client quit message
quit_window: quit message for clients on window machine
quit_other: quit message for clients on other machines
*/
char* create_quit_mess(char* username, int is_window){
	int len;
	char* quit;
	quit = strdup(username);
   	len = strlen(quit);
    quit[len] = ':';

	if(is_window)
    	strcpy(quit + len+1, "/quit\r\n");
    else
		strcpy(quit + len+1, "/quit\n");
	return quit;
}

//Notify other clients when newcomer join the server
void announce_joining(char* message, char* username){
	sprintf(message, "%s has joined the server\n", username);
	add_message_queue(message);
}

//Notify other clients when client leaves the server
void announce_leaving(char* message, char* username){
	sprintf(message, "%s has left the server\n", username);
	for(int i = 0; client_list[i] != NULL && i < MAX_CLIENTS; i++)
		write(client_list[i]->own_sock, message, strlen(message));
}
