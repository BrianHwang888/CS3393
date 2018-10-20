#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define MAX_BUFFER 1024
#define MAX_ARG 64
#define IOSYM "><2>>><<2>>"
#define DELIM " \t\r\n\a"

void read_line(char **line);
void parse_line(char *line, char ***exe_argv);
void parse_cmd(char *line);
void redirect(char **argv);
void exe_cmd(char **exe_argv);
int process_argv(char **exe_argv);

int main(){
	char *prompt;
	size_t prompt_size = MAX_BUFFER;
	char *line = NULL;
	char **exe_argv = NULL;
	int std_in = dup(0);
	int std_out = dup(1);
	int std_err = dup(2);
	int done = 0;
	struct sigaction quit_act;

	if(std_in == -1){
		perror("Failed to dup main stdin\n");
		exit(EXIT_FAILURE);
	}
	if(std_out == -1){
        perror("Failed to dup main stdout\n");
        exit(EXIT_FAILURE);
    }
	if(std_err == -1){
        perror("Failed to dup main stderr\n");
        exit(EXIT_FAILURE);
    }

	quit_act.sa_handler = SIG_IGN;
	sigemptyset(&quit_act.sa_mask);
	quit_act.sa_flags = 0;

	if((prompt = getenv("PS1")) == NULL){
		prompt = getcwd(prompt, prompt_size);
	};

	if(sigaction(SIGQUIT, &quit_act, NULL) == -1)
        perror("Failed to handle SIGQUIT\n");
    if(sigaction(SIGINT, &quit_act, NULL) == -1)
    	perror("Failed to handle SIGINT\n");
	//loop for the shell
	do{
		printf("%s> ", prompt);

		read_line(&line);
		parse_line(line, &exe_argv);
		redirect(exe_argv);
		done = process_argv(exe_argv);

		prompt = getcwd(prompt, prompt_size);
		free(line);
		free(exe_argv);

		//resets the std streams
		if(dup2(std_in, 0) == -1){
			perror("Failed to reset to main stdin\n");
			exit(EXIT_FAILURE);
		}
		if(dup2(std_out, 1) == -1){
			perror("Failed to reset to main stdout\n");
			exit(EXIT_FAILURE);
		}
		if(dup2(std_err, 2) == -1){
			perror("Failed to reset to main stderr\n");
			exit(EXIT_FAILURE);
		}
	}while(!done);

	free(prompt);
	exit(EXIT_SUCCESS);
}

//takes in user input
void read_line(char **line){
	size_t buf_size = MAX_BUFFER;
	char *buffer = malloc(sizeof(char) * MAX_BUFFER);
	if(buffer == NULL){
        fprintf(stderr, "Failed to allocate memory for input\n");
        exit(EXIT_FAILURE);
    }

	if (getline(&buffer, &buf_size, stdin) == -1){
		perror("Failed to read from stdin\n");
		exit(EXIT_FAILURE);
	}
	*line = buffer;
}

//creates arguments for the given command
void parse_line(char *line, char ***exe_argv){
	int cmd_ind = 0;
	int buf_max = MAX_ARG;

	char **cmd_buff = malloc(sizeof(char *) * MAX_ARG);
	if(cmd_buff == NULL){
        fprintf(stderr, "Failed to allocate memory for cmd_argv\n");
        exit(EXIT_FAILURE);
    }

	cmd_buff[cmd_ind] = strtok(line, DELIM);
	cmd_ind++;

	//storing arguments for the command
	while((cmd_buff[cmd_ind] = strtok(NULL, DELIM)) != NULL){
		cmd_ind++;

		//realloc cmd array if the buffer is full
		if(cmd_ind >= buf_max){
			buf_max *= 2;
			cmd_buff = realloc(cmd_buff, sizeof(char *) * buf_max);
			if(cmd_buff == NULL){
				perror("Failed to realloc cmd_buff\n");
				exit(EXIT_FAILURE);
			}
		}
	}
	cmd_buff[cmd_ind] = NULL;

	*exe_argv = cmd_buff;
}

//process the commands with their given arguments
int process_argv(char **exe_argv){
	if(exe_argv[0] == NULL)
		return 0;

	else if(strcmp(exe_argv[0], "exit") == 0)
		return 1;

	//checks if command is cd
	else if(strcmp(exe_argv[0], "cd") == 0){
		if(exe_argv[1] != NULL){
			if(chdir(exe_argv[1]) == -1){
            	fprintf(stderr, "Failed to change to directory: %s\n", exe_argv[1]);
            	exit(EXIT_FAILURE);
        	}
		}
		else{
			if(chdir(getenv("HOME")) == -1){
				perror("Failed to return to home directory\n");
				exit(EXIT_FAILURE);
			}
		}
	}
	else
		exe_cmd(exe_argv);
	return 0;
}

//forks children to exec commands with arguments
void exe_cmd(char **exe_argv){
	pid_t pid;
	int status;

	pid = fork();

	if(pid < 0){
		fprintf(stderr, "Failed to fork process for command: %s\n", exe_argv[0]);
		exit(EXIT_FAILURE);
	}

	if(pid == 0){
		if(execvp(exe_argv[0], exe_argv) == -1){
			fprintf(stderr, "Failed to execute: %s\n", exe_argv[0]);
			exit(EXIT_FAILURE);
		}
	}
	while(waitpid(pid, &status, WUNTRACED) != pid);
}

/*redirects io and modify argument to exec command
redirection includes stdin, stdout, stderr
allows for appending for said streams
*/
void redirect(char **argv){
	int i;
	int fd[3];

	for(i = 0; argv[i] != NULL; i++){
		if(strcmp(argv[i], ">") == 0){
			if((fd[1] = open(argv[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0666)) == -1){
				fprintf(stderr, "Failed to store output in: %s\n", argv[i+1]);
				exit(EXIT_FAILURE);
			}
			if(dup2(fd[1], 1) == -1){
				perror("Failed to dup2 for >\n");
				exit(EXIT_FAILURE);
			}
			if(close(fd[1]) == -1){
				perror("Failed to close file's stdout\n");
				exit(EXIT_FAILURE);
			}
			argv[i] = NULL;
		}
		else if(strcmp(argv[i], "<") == 0){
			puts("is reading");
			if((fd[0] = open(argv[i+1], O_RDONLY)) == -1){
				fprintf(stderr, "Failed to use %s as input\n", argv[i+1]);
				exit(EXIT_FAILURE);
			}
			if(dup2(fd[0], 0) == -1){
				perror("Failed to dup2 for <\n");
				exit(EXIT_FAILURE);
			}
			if(close(fd[0]) == - 1){
				perror("Failed to close file's stdin\n");
				exit(EXIT_FAILURE);
			}
			argv[i] = NULL;
        }
		else if(strcmp(argv[i], "2>") == 0){
			if((fd[2] = open(argv[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0666)) == -1){
				fprintf(stderr, "Failed to store error in: %s\n", argv[i+1]);
				exit(EXIT_FAILURE);
			}
       		if(dup2(fd[2], 2) == -1){
				perror("Failed to dup2 for 2>\n");
				exit(EXIT_FAILURE);
			}
			if(close(fd[2]) == -1){
				perror("Failed to close file's stderr\n");
				exit(EXIT_FAILURE);
			}
            argv[i] = NULL;
		}
		else if(strcmp(argv[i], ">>") == 0){
			if((fd[1] = open(argv[i+1], O_WRONLY | O_CREAT | O_APPEND, 0666)) == -1){
				fprintf(stderr, "Failed to append output in: %s\n", argv[i+1]);
				exit(EXIT_FAILURE);
			}
        	if(dup2(fd[1], 1) == -1){
				perror("Failed to dup2 for >>\n");
				exit(EXIT_FAILURE);
			}
			if(close(fd[1]) == -1){
                perror("Failed to close file's stdout\n");
                exit(EXIT_FAILURE);
            }
			argv[i] = NULL;
		}
		else if(strcmp(argv[i], "<<") == 0){
			if((fd[0] = open(argv[i+1], O_RDONLY)) == -1){
				fprintf(stderr, "Failed to append %s for input\n", argv[i+1]);
				exit(EXIT_FAILURE);
			}
        	if(dup2(fd[0], 0) == -1){
				perror("Failed to dup2 for <<\n");
				exit(EXIT_FAILURE);
			}
			if(close(fd[0]) == -1){
				perror("Failed to close file's stdin\n");
				exit(EXIT_FAILURE);
			}
			argv[i] = NULL;
		}
		else if(strcmp(argv[i], "2>>") == 0){
			if((fd[2] = open(argv[i+1], O_WRONLY | O_CREAT | O_APPEND, 0666)) == -1){
				fprintf(stderr, "Failed to append error to: %s\n", argv[i+1]);
				exit(EXIT_FAILURE);
			}
        	if(dup2(fd[2], 2) == -1){
				perror("Failed to dup2 for 2>>\n");
				exit(EXIT_FAILURE);
			}
			if(close(fd[2]) == -1){
				perror("Failed to close file's stderr\n");
				exit(EXIT_FAILURE);
			}
			argv[i] = NULL;
		}
	}
}
