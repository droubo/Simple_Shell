#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 
#include <fcntl.h>
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/wait.h>

#define MINSIZE 8
#define MAX_DIR_LEN 128
#define MIN_ARGS 12
#define MIN_PIPES 512

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
void remove_blankspaces(char *s) {
	const char* d = s;
	do {
		while (*d == ' ') {
			++d;
		}
	} while (*s++ = *d++);
}

int check_rdr_mode(char *input) {
	if (strstr(input, ">>") != NULL) return 0;
	else if (strchr(input, '>') != NULL) return 1;
	else if (strchr(input, '<') != NULL) return 2;

	return -1;
}
void redirect(char *command, int mode) {

	pid_t pid_1;
	int f;
	int flag = STDOUT_FILENO;
	char *modes[3] = { ">>", ">", "<" };
	char **cmd = malloc(MIN_ARGS * sizeof(char *));
	char **splitted = malloc(MIN_PIPES * sizeof(char *));

	splitter(command, splitted, modes[mode]);
	splitter(splitted[0], cmd, " ");

	if (mode == 0) f = open(splitted[1], O_WRONLY | O_APPEND | O_CREAT, S_IRWXU);
	else if (mode == 1) f = open(splitted[1], O_WRONLY | O_TRUNC | O_CREAT, S_IRWXU);
	else if (mode == 2) {
		f = open(splitted[1], O_RDONLY, S_IWUSR);
		flag = STDIN_FILENO;
	}
	remove_blankspaces(splitted[1]);
	dup2(f, flag);
	close(f);
	execvp(cmd[0], cmd);
	//exit(0);
	return;
}
void execute_commands(char** cmd) {
	int fd[2];
	pid_t pid;
	int fdd = 0;
	int i = 0;
	char *command[MIN_ARGS];
	int mode;
	while (cmd[i] != NULL) {
		mode = check_rdr_mode(cmd[i]);
		pipe(fd);
		printf("Mode:%d\n", mode);
		if ((pid = fork()) == -1) {
			perror("fork");
			exit(1);
		}
		else if (pid == 0) {
			dup2(fdd, 0);
			if ((cmd[i + 1]) != NULL) {
				dup2(fd[1], 1);
			}
			close(fd[0]);
			if (mode != -1) {
				redirect(cmd[i], mode);
			}
			else {
				splitter(cmd[i], command, " ");
				execvp(command[0], command);
			}
			exit(0);
		}
		else {
			wait(NULL);
			close(fd[1]);
			fdd = fd[0];
			i++;
		}
	}
}
int splitter(char* str, char** splitted, char* splitter)
{
	int i = 0;
	char* rest = strdup(str);
	int curr_size = MIN_PIPES;
	while ((splitted[i] = strtok_r(rest, splitter, &rest))) {
		i++;
		if (i > curr_size) {
			splitted = realloc(splitted, sizeof(char *) * (MIN_PIPES + i));
			curr_size = MIN_PIPES + i;
		}
	}
	return i;
}
int mode(char *input) {

	char **splitted = malloc(MIN_PIPES * sizeof(char *));
	int pipe_mode = strchr(input, '|') != NULL;
	int exit_mode = !strcmp(input, "exit");
	int rdr_mode = check_rdr_mode(input);

	splitter(input, splitted, " ");

	if (exit_mode) exit(0);

	else if (!strcmp(splitted[0], "cd")) {
		change_directory(splitted[1]);
		return 0;
	}
	else if (!strcmp(splitted[0], "setenv")) {
		if (setenv(splitted[1], splitted[2], 1) == -1) {
			printf("Invalid command\n");
			return 1;
		}
	}
	else if (!strcmp(splitted[0], "unsetenv")) {
		if (unsetenv(splitted[1]) == -1) {
			printf("Invalid command\n");
			return 1;
		}
	}
	else if (!strcmp(splitted[0], "env")) {
		char *path = getenv("PATH");
		char *home = getenv("HOME");

		printf("PATH=%s\n", (path != NULL) ? path : "NULL");
		printf("HOME=%s\n", (home != NULL) ? home : "NULL");
	}
	else {
		splitter(input, splitted, "|");
		execute_commands(splitted);
	}
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
int take_user_input(char** input) {
	char *buf = malloc(MINSIZE);
	char c;
	int input_size = MINSIZE, i = 0;
	if (print_prompt() == 1) return 1;

	clean_string(buf, MINSIZE);
	while ((c = getchar()) != '\n' && c != EOF) {
		buf[i++] = c;
		if (i >= input_size) {
			input_size += MINSIZE;
			buf = realloc(buf, input_size);
		}
	}

	if (input_size > MINSIZE) *input = realloc(*input, input_size);
	clean_string(*input, input_size);
	if (strlen(buf) != 0) {
		strcpy(*input, buf);
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
		if (take_user_input(&input) == 0) {
			mode(input);
			free(input);
		}
	}
}