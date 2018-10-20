#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

extern char **environ;

int count_pairs(const char **argu);
void set_exe(char **argu, char **cmd, char **argv);
void mod_environ(char **argv, int num_pairs, char **environ);
void make_environ(char **argv, int num_pairs, char **environ);

int main(int argc, char *argv[]){
	int env_index = 0;
	int index;
	int num_pairs;
	int siz_exe_argv;
	char *input;
	char *cmd;
	char **exe_argv;
	char **new_environ;
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

		index = 2;
		num_pairs = count_pairs((argv+index));
		new_environ = malloc(sizeof(char*) * num_pairs);

		make_environ((argv + index), num_pairs, new_environ);

		if(argv[num_pairs + 2] == NULL){
			for(env_index = 0; new_environ[env_index] != NULL; env_index++){
                                puts(new_environ[env_index]);
			}
/*			for(index = 0; new_environ[index] != NULL; index++)
                                free(new_environ[index] );
                        free(new_environ);*/
		}

		//executes the command with the given arguments and new environment
		else{
			environ = new_environ;
			index = num_pairs + 2;
			siz_exe_argv = argc - num_pairs - 3;

			input = malloc(sizeof(char) * (strlen(argv[index]) + 1));
			if(input == NULL){
				perror("Not enough space for input");
				exit(EXIT_FAILURE);
			}
			input = strcpy(input, argv[index]);

			//sets up path and execve array
			exe_argv = malloc(sizeof(char) * siz_exe_argv);
			set_exe((argv + index), &cmd, exe_argv);

			free(input);

			execvp(cmd, exe_argv);
			if(errno | 0){
				fprintf(stderr, "Error in execution of command: %s\n", strerror(errno));
/*				for(index = 0; exe_argv[index] != NULL; index++)
                                	free(exe_argv[index]);
                        	free(exe_argv);
				for(index = 0; new_environ[index] != NULL; index++)
                                	free(new_environ[index]);
                        	free(exe_argv);*/
				exit(EXIT_FAILURE);
			}
		}
	}
	/*
	modifies the environment array with the key value pairs provided
	in the input and appends pairs to the array if they don't exist
	*/
	else if(strcmp(argv[1], "-i") != 0){
		int length = 0;
		index = 1;
		/*
		loops through the input for key value pairs and updates the
		environ array accordingly
		*/
		num_pairs = count_pairs((argv+index));
		for(env_index = 0; environ[env_index] != NULL; env_index++){
			length++;
		}

		new_environ = malloc(sizeof(char*) * (length + num_pairs));

		for(env_index = 0; environ[env_index] != NULL; env_index++){
			new_environ[env_index] = malloc(sizeof(char)* (strlen(environ[index]) + 1));
			new_environ[env_index] = environ[env_index];
		}

		mod_environ((argv+index), num_pairs, new_environ);

		//prints out the modified environ if there are no commands given
		if(argv[num_pairs + 1] == NULL){
			for(index = 0; new_environ[index] != NULL; index++){
	                        puts(new_environ[index]);
			}
/*			for(index = 0; new_environ[index] != NULL; index++)
                                free(new_environ[index]);
                        free(new_environ);*/
		}

		//executes the command in the modified environ with the given arguments
		else{
			environ = new_environ;
			siz_exe_argv = argc - num_pairs - 2;
			index = num_pairs + 1;

			input = malloc(sizeof(char) * (strlen(argv[index]) + 1));
			input = strcpy(input, argv[index]);

			exe_argv = malloc(sizeof(char*) * (siz_exe_argv + 1));
			set_exe((argv + index), &cmd, exe_argv);


			free(input);
			execvp(cmd, exe_argv);
			if(errno | 0){
				fprintf(stderr, "Error in execution of command: %s\n", strerror(errno));
/*				for(index = 0; exe_argv[index] != NULL; index++)
                                	free(exe_argv[index]);
                        	free(exe_argv);
				for(index = 0; new_environ[index] != NULL; index++)
                                	free(new_environ[index]);
                        	free(new_environ);*/
				exit(EXIT_FAILURE);
			}
		}
	}
	return 0;
}

//counts the number of key value pairs in the array
int count_pairs(const char **argu){
	char *input;
	char *key;
	int num_pairs = 0;
	int argu_index = 0;

	input = (char *)malloc(sizeof(char) * (strlen(argu[argu_index]) + 1));
        input = strcpy(input, argu[argu_index]);
        key = strtok(input, "=");
	if(key == NULL){
		return 0;
	}
	for(argu_index=0; argu[argu_index] != NULL; argu_index++){
        	input = (char *)realloc(input, sizeof(char) * (strlen(argu[argu_index]) + 1));
                input = strcpy(input, argu[argu_index]);
                key = strtok(input, "=");
		key = strtok(NULL, "=");
		if(key != NULL)
			num_pairs++;
	}
	free(input);
	return num_pairs;
}

//sets the path and arguments to be executed
void set_exe(char **given_args, char **cmd, char **argv){
	int index = 0;
	*cmd = given_args[index];

	for(index = 0; given_args[index] != NULL; index++){
		argv[index] = malloc(sizeof(char) * (strlen(given_args[index]) + 1));
		argv[index] = given_args[index];
	}
	argv[index] = NULL;
}

//creates new environ for -i option
void make_environ(char **argv, int num_pairs, char **environ){
	int index;
	for(index = 0; index < num_pairs; index++){
		environ[index] = malloc(sizeof(char) * (strlen(argv[index]) + 1));
		environ[index] = argv[index];
	}
	environ[index] = NULL;
}

//changes environ based on the given key value pairs
void mod_environ(char **argv, int num_pairs, char **environ){
	int index;
	int env_index = 0;
	char *input = malloc(sizeof(char) * (strlen(*argv) + 1));
	char *environ_val;
	char *key_input;
	char *key_environ;

	/*
	checks to see if environ is empty
	if environ is empty environ_val memory allocated for first argv
	o/w memory allocated for first environ value
	*/
	if(*environ != NULL)
		environ_val = malloc(sizeof(char) * (strlen(*environ) + 1));
	else
		environ_val = malloc(sizeof(char) * (strlen(*argv) + 1));

	for(index = 0; index < num_pairs; index++){
		input = realloc(input, sizeof(char) * (strlen(argv[index]) + 1));
                input = strcpy(input, argv[index]);
		key_input = strtok(input, "=");

		if(environ[env_index] != NULL){
                	environ_val = realloc(environ_val, sizeof(char) * (strlen(environ[env_index]) + 1));
                	environ_val = strcpy(environ_val, environ[env_index]);
        	}
		else
			environ_val = strcpy(environ_val, argv[index]);

		key_environ = strtok(environ_val, "=");

		for(env_index = 0; environ[env_index] != NULL && strcmp(key_environ, key_input) != 0; env_index++){
			environ_val = realloc(environ_val, sizeof(char) * (strlen(environ[env_index]) + 1));
			environ_val = strcpy(environ_val, environ[env_index]);
			key_environ = strtok(environ_val, "=");
		}
		env_index--;
		if(environ[env_index+1] == NULL && strcmp(key_environ, key_input) != 0){
			environ[env_index+1] = malloc(sizeof(char) * (strlen(argv[index]) + 1));
			environ[env_index+1] = argv[index];
			environ[env_index + 2] = malloc(sizeof(NULL));
			environ[env_index + 2] = NULL;
		}
		else{
			if(env_index+1 == 0){
				if(strlen(environ[env_index+1]) < strlen(argv[index]))
					environ[env_index+1] = realloc(environ[env_index+1], sizeof(char) * (strlen(argv[index]) + 1));
				environ[env_index+1] = strcpy(environ[env_index+1], argv[index]);
			}
			else{
				if(strlen(environ[env_index]) < strlen(argv[index]))
                                        environ[env_index] = realloc(environ[env_index], sizeof(char) * (strlen(argv[index]) + 1));
				environ[env_index] = strcpy(environ[env_index], argv[index]);
			}
		}
		env_index = 0;
	}
	free(environ_val);
	free(input);
}
