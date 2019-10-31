#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/*
 * Antonis Ntroumpogiannis
 * csd4014@csd.uoc.gr
 */

void execute_commands(char **cmd);

char ** splitter(char *str, char **splitted, char *splitter)
{
	int i = 0;
	char *rest = strdup(str);
	char *tmp;
	while(splitted[i++] = strtok_r(rest, splitter, &rest)){
		splitted = realloc(splitted, sizeof(char*) * (i + 1));
	}
	return splitted;
}

void change_directory(char *path)
{
	if (path == NULL)
	{
		printf("No path entered!\n");
		return;
	}
	if (chdir(path) != 0)
	{
		printf("Path not found!\n");
	}
	return;
}
int check_rdr_mode(char *input)
{
	int flag = 0;
	if (strstr(input, ">>") != NULL)
		flag += 1;
	else if (strchr(input, '>') != NULL)
		flag += 2;
	else if (strchr(input, '<') != NULL)
		flag += 3;

	switch (flag)
	{
	case 1:
		return 0;
	case 2:
		return 1;
	case 3:
		return 2;
	case 4:
		return 3;
	case 5:
		return 4;
	default:
		return -1;
	}
}
void redirect(char *command, int mode)
{

	pid_t pid_1;
	int f;
	int flag = STDOUT_FILENO;
	char *modes[3] = {">>", ">", "<"};
	char **splitted = malloc(sizeof(char *));

	if (mode == 4)
		mode = 0;
	if (mode == 5)
		mode = 1;

	splitted = splitter(command, splitted, modes[mode]);

	char *tmp_1 = splitted[1], *tmp_2 = splitted[1];
	do
	{
		if (*tmp_2 != ' ')
			*tmp_1++ = *tmp_2;
	} while (*tmp_2++);

	if (mode == 0)
		f = open(splitted[1], O_WRONLY | O_APPEND | O_CREAT, S_IRWXU);
	else if (mode == 1)
		f = open(splitted[1], O_WRONLY | O_TRUNC | O_CREAT, S_IRWXU);
	else if (mode == 2)
	{
		f = open(splitted[1], O_RDONLY, S_IWUSR);
		flag = STDIN_FILENO;
	}
	dup2(f, flag);
	close(f);
	splitted[1] = NULL;
	execute_commands(splitted);
	return;
}
void execute_commands(char **cmd)
{
	pid_t pid;
	int mode, tmp_fd = 0, i = 0, file_descr[2];
	char **command = malloc(sizeof(char *));
	while (cmd[i] != NULL)
	{
		mode = check_rdr_mode(cmd[i]);
		pipe(file_descr);
		pid = fork();
		if (pid == 0)
		{
			close(file_descr[0]);
			dup2(tmp_fd, 0);
			if ((cmd[i + 1]) != NULL)
			{
				dup2(file_descr[1], 1);
			}
			if (mode != -1)
			{
				redirect(cmd[i], mode);
			}
			else
			{
				command = splitter(cmd[i], command, " ");
				if(execvp(command[0], command) < 0){
					printf("Error, %s command invalid!\n", command[0]);
				}
			}
			exit(0);
		}
		else if (pid == -1)
		{
			exit(1);
		}
		else
		{
			close(file_descr[1]);
			tmp_fd = file_descr[0];
			wait(NULL);
			i++;
		}
	}
}
int mode(char *input)
{

	char **splitted = malloc(sizeof(char *));
	char *modes[5] = {"exit", "cd", "setenv", "unsetenv", "env"};
	int mode = 5, i;
	splitted = splitter(input, splitted, " ");

	for (i = 0; i < 5; i++)
	{
		if (!strcmp(modes[i], splitted[0]))
			mode = i;
	}

	switch (mode)
	{
	case 0:
		exit(0);
	case 1:
	{
		change_directory(splitted[1]);
		return 0;
	}
	case 2:
	{
		if (setenv(splitted[1], splitted[2], 1) == -1)
		{
			printf("Error, invalid command\n");
			return 1;
		}
	}
	break;
	case 3:
		if (unsetenv(splitted[1]) == -1)
		{
			printf("Error, invalid command\n");
			return 1;
		}
		break;
	case 4:
	{
		char *path = getenv("PATH");
		char *home = getenv("HOME");
		printf("PATH=%s\n", (path != NULL) ? path : "NULL");
		printf("HOME=%s\n", (home != NULL) ? home : "NULL");
	}
	break;
	case 5:
	{
		splitted = splitter(input, splitted, "|");
		execute_commands(splitted);
	}
	}
}

int print_prompt()
{
	char *user = getlogin();
	char *dir = getcwd(NULL, 0);
	if (user == NULL || dir == NULL)
	{
		return 1;
	}
	printf("%s@cs345sh~%s$ ", user, dir);
	return 0;
}
int take_user_input(char **input)
{
	char *buf = malloc(sizeof(char));
	char c;
	int input_size = sizeof(char), i = 0;
	if (print_prompt() == 1)
		return 1;

	while ((c = getchar()) != '\n' && c != EOF)
	{
		buf[i++] = c;
		if (i >= input_size)
		{
			input_size += sizeof(char);
			buf = realloc(buf, sizeof(char) * input_size);
		}
	}
	buf[i] = '\0';
	if (input_size > sizeof(char))
		*input = realloc(*input, sizeof(char) * input_size);
	if (strlen(buf) != 0)
	{
		memcpy(*input, buf, input_size);
		free(buf);
		return 0;
	}
	else
	{
		free(buf);
		return 1;
	}
}

int main()
{
	char *input;
	int i = 0;
	while (1)
	{
		input = malloc(sizeof(char) * sizeof(char));
		if (take_user_input(&input) == 0)
		{
			mode(input);
			free(input);
		}
	}
}