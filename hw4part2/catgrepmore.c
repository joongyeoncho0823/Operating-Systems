#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <setjmp.h>

char *pattern;
int status1, status2;
int totalbytesread, totalfiles;
int val;
jmp_buf env_buffer;
int i;

void do_function(char *inputfile)
{
    printf("do function\n");
    int pp1[2];
    int pp2[2];
    if (pipe(pp1) < 0)
    {
        fprintf(stderr, "Error with pipe pp1: %s\n", strerror(errno));
        exit(-1);
    }
    if (pipe(pp2) < 0)
    {
        fprintf(stderr, "Error with pipe pp2: %s\n", strerror(errno));
        exit(-1);
    }

    int pidgrep;
    pidgrep = fork();
    printf("forked grep\n");
    if (pidgrep == 0)
    {
        if (dup2(pp1[0], STDIN_FILENO) < 0)
        {
            fprintf(stderr, "Error with dup2: %s\n", strerror(errno));
            exit(-1);
        }
        if (close(pp1[0]) < 0)
        {
            fprintf(stderr, "Error with close READ of pp1: %s\n", strerror(errno));
            exit(-1);
        }
        if (close(pp1[1]) < 0)
        {
            fprintf(stderr, "Error with close WRITE of pp1: %s\n", strerror(errno));
            exit(-1);
        }
        if (dup2(pp2[1], STDOUT_FILENO) < 0)
        {
            fprintf(stderr, "Error with dup2: %s\n", strerror(errno));
            exit(-1);
        }
        if ((close(pp2[0])) < 0)
        {
            fprintf(stderr, "Error with close READ of pp2 in grep: %s\n", strerror(errno));
            exit(-1);
        }
        if (close(pp2[1]) < 0)
        {
            fprintf(stderr, "Error with close WRITE of pp2: %s\n", strerror(errno));
            exit(-1);
        }
        printf("Exec grep\n");
        execlp("grep", "grep", pattern, NULL);
        fprintf(stderr, "error in exec grep: %s\n", strerror(errno));
        exit(0);
    }
    int pidmore;
    pidmore = fork();
    if (pidmore == 0)
    {
        if (dup2(pp2[0], STDIN_FILENO) < 0)
        {
            fprintf(stderr, "Error with dup2: %s\n", strerror(errno));
            exit(-1);
        }
        if (close(pp1[0]) < 0)
        {
            fprintf(stderr, "Error with close READ of pp1: %s\n", strerror(errno));
            exit(-1);
        }
        if (close(pp1[1]) < 0)
        {
            fprintf(stderr, "Error with close WRITE of pp1: %s\n", strerror(errno));
            exit(-1);
        }
        if ((close(pp2[0])) < 0)
        {
            fprintf(stderr, "Error with close READ of pp2 in more: %s\n", strerror(errno));
            exit(-1);
        }
        if (close(pp2[1]) < 0)
        {
            fprintf(stderr, "Error with close WRITE of pp2: %s\n", strerror(errno));
            exit(-1);
        }
        printf("Exec more\n");
        execlp("more", "more", NULL);
        fprintf(stderr, "error in exec more: %s\n", strerror(errno));
        exit(0);
    }
    if (pidgrep != 0 && pidmore != 0) // parent
    {
        if (close(pp1[0]) < 0)
        {
            fprintf(stderr, "Error with close READ of pp1: %s\n", strerror(errno));
            exit(-1);
        }
        if (close(pp2[0]) < 0)
        {
            fprintf(stderr, "Error with close READ of pp2 in parent: %s\n", strerror(errno));
            exit(-1);
        }
        if (close(pp2[1]) < 0)
        {
            fprintf(stderr, "Error with close WRITE of pp2: %s\n", strerror(errno));
            exit(-1);
        }

        int fd;
        if ((fd = open(inputfile, O_RDONLY, 0666)) < 0)
        {
            fprintf(stderr, "error opening inputfile: %s\n", strerror(errno));
            exit(-1);
        }

        char *buf = malloc(4096);
        int bytesread;
        while ((bytesread = read(fd, buf, 4096)) > 0)
        {
            if ((write(pp1[1], buf, bytesread)) < 0)
            {
                fprintf(stderr, "Error writing to pp1 from inputfile: %s\n", strerror(errno));
                exit(-1);
            }
            totalbytesread += bytesread;
        }
        if (close(fd) < 0)
        {
            fprintf(stderr, "Could not close fd: %s\n", strerror(errno));
            exit(-1);
        }
        if (close(pp1[1]) < 0)
        {
            fprintf(stderr, "Could not close WRITE OF pp1: %s\n", strerror(errno));
            exit(-1);
        }

        waitpid(pidgrep, &status1, 0);
        waitpid(pidmore, &status2, 0);
        printf("grep status: %d\n more status: %d\n", status1, status2);
    }
}

void sig_handler2(int signum) // SIGUSR2
{
    fprintf(stderr, "Got signal: %d\n", signum);
    fprintf(stderr, "SIGUSR2 received, moving on to file #%d (if exists)\n", i);
    longjmp(env_buffer, 0);
}
void sig_handler1(int signum) // SIGUSR1
{
    fprintf(stderr, "Got signal: %d\n", signum);
    fprintf(stderr, "Total number of files: %d\n", totalfiles);
    fprintf(stderr, "Total number of bytes: %d\n", totalbytesread);
}
int main(int argc, char *argv[])
{
    signal(SIGUSR2, sig_handler2);
    signal(SIGUSR1, sig_handler1);
    pattern = argv[1];
    for (i = 2; i < argc; i++)
    {
        val = setjmp(env_buffer);
        if (val == 0)
        {
            // printf("Not jumped\n");

            do_function(argv[i]);
            totalfiles++;
        }
        else
        {
            printf("Jumped!\n");
        }
    }
    fprintf(stderr, "Total number of files: %d\n", totalfiles);
    fprintf(stdout, "Read %d bytes total\n", totalbytesread);

    return 0;
}