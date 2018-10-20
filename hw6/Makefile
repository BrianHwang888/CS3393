all: server client

client: client.c
	gcc -o client -std=c99 -Wall -g -D_BSD_SOURCE -pthread client.c

server: server.c
	gcc -o server -std=c99 -Wall -g -D_BSD_SOURCE server.c -pthread

clean:
	rm -f client
	rm -f server
