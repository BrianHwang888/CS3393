#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

extern char** environ;

int main(int argc, char* argv[]){

	char* entry, *key;
	entry = strdup(*environ);
	key = strtok(entry, "=");
	while(strcmp(argv[1], key) != 0 && *(++environ) != NULL){
		entry = strdup(*environ);
		key = strtok(entry, "=");
	}
	if(*environ != NULL)
		printf("%s", *environ);
}
