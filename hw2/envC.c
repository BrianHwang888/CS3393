#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <unistd.h>

extern char **environ;
int index = 2;

void set_exe(char *input, char **argu, char **path, int siz_argv, char **argv);
int count_pairs(const char **argu);

int main(int argc, char *argv[]){
	int env_index = 0;
	int exe_index = 0;
	int siz_exe_argv;
	char *input;
	char *path;
	char **exe_argv;
	/*
	prints out all of the environment variables and their values
	in the environment array 
	*/
	if(argc == 1){
		//goes through the environment array and prints their value
		for(env_index = 0; environ[env_index] != NULL; env_index++)
			puts(environ[env_index]);
	}
	/*
	creates a new array with the data stored in the environmet array
	and applies the modified data
	*/
	else if(strcmp(argv[1], "-i") == 0){
		char **new_environ;
		int num_pairs;

		num_pairs = count_pairs(argv);
		new_environ = malloc(sizeof(char) * (argc - 2));
		siz_exe_argv = argc - num_pairs - 3;
		for(index = 2; index < num_pairs; index++){
                        new_environ[env_index] = argv[index];
                        env_index++;
                }
		input = malloc(sizeof(char) * (strlen(argv[index]) + 1));
		input = strcpy(input, argv[index]);
//		if(strtok(input,"=") == NULL)
//                        set_exe(input, argv + index, &path, siz_exe_argv, exe_argv);
		printf("%s %d %s\n", input, index, argv[index]);
		free(input);
		for(index = 0; new_environ[index] != NULL; index++)
			puts(new_environ[index]);
	}
	/*
	modifies the environment variable specified in argv[1] with the
	data stored in it
	*/
	else if(strcmp(argv[1], "-i") != 0){
/*		char *sub_part;
		int len
		while(environ[env_index] != NULL && environ[])
		for(index = 0; environ[index] != sub_part; index++)
                        if(index == length){
                                printf("Error: can't find the variable in the array\n");
                                exit(-1);
                        }*/

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
                input = (char *)realloc(input, sizeof(char) * (strlen(argu[index]) + 1));
                input = strcpy(input, argu[index]);
                key = strtok(input, "=");
                index++;
                num_pairs++;
        }
        free(input);
        return num_pairs;
}
