#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 
#include <fcntl.h>
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/wait.h>

#define MINSIZE 16
#define MAX_DIR_LEN 128
#define MIN_ARGS 6

void change_directory(char *path) {
	if (path == NULL) {
		printf("No path entered!\n");
		return;
	}
	if (chdir(path) != 0) {
		printf("Path not found!\n");
	}
	return;
}
void pipe_exec(char** arg, char** args_piped) {

	int fd[2];
	pid_t pr1, pr2;

	if (pipe(fd) < 0) {
		printf("\nError");
		return;
	}
	pr1 = fork();
	if (pr1 < 0) {
		return;
	}

	if (pr1 == 0) {

		close(fd[0]);

		dup2(fd[1], STDOUT_FILENO);

		close(fd[1]);

		if (execvp(arg[0], arg) < 0) {
			printf("\nCommand 1 not found.");
			exit(0);
		}
	}
	else {
		pr2 = fork();

		if (pr2 < 0) {
			return;
		}

		if (pr2 == 0) {
			close(fd[1]);

			dup2(fd[0], STDIN_FILENO);

			close(fd[0]);

			if (execvp(args_piped[0], args_piped) < 0) {
				printf("\nCommand 2 not found");
				exit(0);
			}
		}
		else {
			wait(NULL);
			wait(NULL);
		}
	}
}
void execute_simple_command(char** arg) {
	pid_t child = fork();
	if (child == 0) {
		if (execvp(arg[0], arg) < 0) {
			printf("\nInvalid Command");
		}
		exit(0);
	}
	else if (child == -1) return;
	else {
		wait(NULL);
		return;
	}
}

void format_arguments(char* inpt, char** arg)
{
	int i = 0;
	char *str = inpt;
	while ((arg[i++] = strtok_r(str, " " , &str)) != NULL) {
		if (i > MIN_ARGS) *arg = realloc(*arg, i + MIN_ARGS);
	}
}

int split_pipe(char* str, char** stripped)
{
	int i = 0;
	char* rest = str;
	while ((stripped[i] = strtok_r(rest, "|", &rest))) {
		printf("%s\n", stripped[i]);
		i++;
		if (i > 32) break;
	}
	return 0;
}

int mode(char *input){

	char  *args[MIN_ARGS];
	char *stripped[32];
	int pipe_mode = strchr(input, '|') != NULL;
	int exit_mode = !strcmp(input, "exit");
	int rdr_mode = (strchr(input, '>') != NULL || strchr(input, '<') != NULL);

	format_arguments(input, args);

	if (exit_mode) exit(0);

	else if (!strcmp(args[0], "cd")) {
		change_directory(args[1]);
		return 0;
	}
	else if (pipe_mode) {
		printf("enter");
		split_pipe(input, stripped);
		return 0;
	}
	else if (rdr_mode) {
		printf("hello\n");
		return 0;
	}
	else {
		printf("\n%s\n", input);	
		execute_simple_command(args);
		return 0;
	}
}


int execute_pipe() {

}
int clean_string(char* str, int size) {
	int i = 0;
	for (i = 0; i < size; i++) str[i] = '\0';
	return 0;
}
int print_prompt() {
	char *user = getlogin();
	char *dir = malloc(MAX_DIR_LEN);
	getcwd(dir, MAX_DIR_LEN);
	if (user == NULL || dir == NULL) {
		return 1;
	}
	printf("%s@cs345sh~%s$ ", user, dir);
	return 0;
}
int take_user_input(char* input) {
	char *buf = malloc(MINSIZE);
	char c;
	int input_size = MINSIZE, i=0;
	if (print_prompt() == 1) return 1;

	clean_string(buf, MINSIZE);
	clean_string(input, MINSIZE);
	while ((c = getchar()) != '\n' && c != EOF) {
		buf[i++] = c;
		if (i >= input_size) {
			input_size += MINSIZE;
			buf = realloc(buf, input_size);
		}
	}

	if(input_size > MINSIZE) input = realloc(input, input_size);

	if (strlen(buf) != 0) {
		strcpy(input, buf);
		free(buf);
		return 0;
	}
	else {
		free(buf);
		return 1;
	}
}

int main() {
	char *input;
	int i = 0;
	while (1) {
		input = malloc(MINSIZE);
		if (take_user_input(input) == 0) {
			mode(input);
			free(input);
		}
	}
}