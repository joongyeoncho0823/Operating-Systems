#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <pwd.h>
#include <errno.h>
#include <sys/resource.h>
#include <sys/times.h>
#include <fcntl.h>

#include <linux/limits.h>
#define PATH_MAX 4096 /* # chars in a path name including nul */

#define MAX_SIZE_CMD 256
#define MAX_SIZE_ARG 16

char cmd[MAX_SIZE_CMD];   // string holder for the command
char *argv[MAX_SIZE_ARG]; // an array for command and arguments
pid_t pid;                // global variable for the child process ID
int status = 0;
int saved_stdout, saved_stderr, saved_stdin;
int out_flag = -1;
int err_flag = -1;
int in_flag = -1;

char *strremove(char *str, const char *sub)
{
    size_t len = strlen(sub);
    if (len > 0)
    {
        char *p = str;
        // Remove substring from string: https: // stackoverflow.com/questions/47116974/remove-a-substring-from-a-string-in-c
        while ((p = strstr(p, sub)) != NULL)
        {
            memmove(p, p + len, strlen(p + len) + 1);
        }
    }
    return str;
}

void get_cmd()
{
    fgets(cmd, MAX_SIZE_CMD, stdin);

    if ((strlen(cmd) > 0) && (cmd[strlen(cmd) - 1] == '\n'))
    {
        cmd[strlen(cmd) - 1] = '\0';
    }

    if (!strcmp("", cmd)) // eof
    {
        return;
    }

    char *token;

    token = strtok(cmd, " ");

    /* walk through other tokens */
    argv[0] = token;

    int idx;
    idx = 1;
    while ((argv[idx] = strtok(NULL, " ")) != NULL)
    {
        idx++;
    }
    argv[idx] = NULL;

    char *possible_redirections[] = {"2>>", ">>", "2>", ">", "<"};

    int j;
    int curr_idx = 0;
    char *cur;
    for (cur = argv[curr_idx]; cur != NULL; cur = argv[++curr_idx])
    {
        for (j = 0; j < sizeof(possible_redirections) / sizeof(possible_redirections[0]); j++)
        {
            if (strstr(argv[curr_idx], possible_redirections[j]) != NULL)
            {
                int fd;
                char *target_file = strremove(argv[curr_idx], possible_redirections[j]);
                if (j == 0 || j == 1)
                {
                    fd = open(target_file, O_WRONLY | O_CREAT | O_APPEND, 0666);
                }
                else if (j == 2 || j == 3)
                {
                    fd = open(target_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
                }
                else
                {
                    fd = open(target_file, O_RDONLY, 0666);
                }

                if (j == 0 || j == 2)
                {
                    dup2(fd, STDERR_FILENO);
                }
                else if (j == 1 || j == 3)
                {
                    saved_stdout = dup(STDOUT_FILENO);
                    dup2(fd, STDOUT_FILENO);
                    out_flag = 1;
                }
                else
                {
                    dup2(fd, STDIN_FILENO);
                }
                if (close(fd) == -1)
                {
                    fprintf(stderr, "Error closing extra reference to I/0 during redirection: %s\n", strerror(errno));
                }
            }
        }
    }
}

void handle_redirections()
{
    if (out_flag != -1)
    {
        dup2(saved_stdout, 1);
        close(saved_stdout);
        out_flag = -1;
    }
    if (err_flag != -1)
    {
        dup2(saved_stderr, 2);
        close(saved_stderr);
        err_flag = -1;
    }
    if (in_flag != -1)
    {
        dup2(saved_stdin, 0);
        close(saved_stdin);
        in_flag = -1;
    }
}

void get_commands()
{
    while (1)
    {
        get_cmd();

        if (!strcmp("", cmd)) // eof
        {
            return;
        }

        if (!strcmp("#", argv[0])) // # (comment)
            continue;
        if (!strcmp(argv[0], "cd")) // cd
        {
            if (argv[1] == NULL)
            {
                struct passwd *pw = getpwuid(getuid());

                argv[1] = pw->pw_dir;
            }
            chdir(argv[1]);
            continue;
        }
        else if (!strcmp(argv[0], "pwd")) // pwd
        {
            char cwd[PATH_MAX];
            if (getcwd(cwd, sizeof(cwd)) != NULL)
            {
                printf("Current working dir: %s\n", cwd);
            }
            else
            {
                perror("getcwd() error");
                return;
            }
            continue;
        }
        else if (!strcmp(argv[0], "exit")) // exit
        {
            if (!argv[1])
                exit(status);
            else
                exit(atoi(argv[1]));
        }

        clock_t start, end;
        struct tms start_buff, end_buff;

        start = times(&start_buff);
        int pid = fork(); // fork child

        if (pid == 0)
        { // Child
            if (execvp(argv[0], argv) == -1)
                exit(-1);
        }
        else
        { // Parent

            struct rusage ru;
            wait3(&status, 0, &ru);
            end = times(&end_buff);
            long clocktick;
            clocktick = sysconf(_SC_CLK_TCK);

            handle_redirections();

            if (status != 0)
            {
                fprintf(stderr, "Child process %d exited with return value %d\n", pid, status >> 8);
            }
            else if (WIFSIGNALED(status))
            {
                fprintf(stderr, "Child process %d exited with signal %d\n", pid, WTERMSIG(status));
            }
            else
            {
                fprintf(stdout, "Child process %d exited normally\n", pid, status);
            }
            fprintf(stderr, "Real: %fs, User: %ld.%.6ds Sys: %ld.%.6ds\n", (end - start) / (double)clocktick, ru.ru_utime.tv_sec, ru.ru_utime.tv_usec, ru.ru_stime.tv_sec, ru.ru_stime.tv_usec);
        }
    }
}

int main()
{
    get_commands();
    return status;
}