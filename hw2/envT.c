#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern char **environ;

int count_pairs(const char **argu);

int index = 2;

int main(int argc, char *argv[]){
	int exe_index = 0;
	int env_index = 0;

	if(argc == 1){
		for(env_index = 0; environ[env_index] != NULL; env_index++)
                        puts(environ[env_index]);
	}

	else if(strcmp(argv[1], "-i") == 0){
                char **new_environ;
                int num_pairs;

		num_pairs = count_pairs(argv);
		new_environ = (char *)malloc(sizeof(char) * num_pairs + 1);
		while(env_index != num_pairs){
                        new_environ[env_index] = malloc(sizeof(char) * (strlen(argv[index]) + 1));
                        new_environ[env_index] = argv[index];
                        env_index++;
                        index++;
                }
		for(env_index = 0; new_environ[env_index] != NULL; env_index++)
                                puts(new_environ[env_index]);
	}
}


int count_pairs(const char **argu){
	char *input;
	char *key;
	int num_pairs = 0;

	input = (char *)malloc(sizeof(char) * (strlen(argu[index]) + 1));
        input = strcpy(input, argu[index]);
        key = strtok(input, "=");

        while(key != argu[index] && argu[index] != NULL){
                input = (char *)realloc(input, sizeof(char) * (strlen(argu[index])));
                input = strcpy(input, argu[index]);
                key = strtok(input, "=");
                index++;
                num_pairs++;
        }
        return num_pairs;
}
