#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <getopt.h>
#include <setjmp.h>

char *p;
char *pattern, *current_inputfile;
int num_opts;
int cflag, cval;
int jump_val;
jmp_buf env_buffer;
int err;
int pattern_size;

void parse_arguments(int argc, char *argv[])
{
    int c;
    int fd;
    struct stat sb;
    while ((c = getopt(argc, argv, "c:p:")) != 1)
    {
        switch (c)
        {
        case 'c':
            cflag = 1;
            cval = atoi(optarg);
            num_opts++;
            break;
        case 'p':
            num_opts++;
            if ((fd = open(optarg, O_RDONLY)) < 0)
            {
                fprintf(stderr, "Error opening pattern file: %s\n", strerror(errno));
                exit(-1);
            }
            if (fstat(fd, &sb) == -1)
            {
                fprintf(stderr, "Error with fstat: %s\n", strerror(errno));
                exit(-1);
            }
            pattern_size = sb.st_size;
            if ((pattern = mmap(NULL, pattern_size, PROT_READ, MAP_PRIVATE, fd, 0)) == MAP_FAILED)
            {
                perror("mmap pattern from file failed\n");
                exit(-1);
            }
            if (close(fd) < 0)
            {
                fprintf(stderr, "error closing fd: %s\n", strerror(errno));
                err = 1;
            }
            break;
        default:
            return;
        }
    }
}

void signalhandler(int signum)
{
    fprintf(stderr, "SIGBUS received while processing file %s", current_inputfile);
    longjmp(env_buffer, 0);
}

int main(int argc, char *argv[])
{
    signal(SIGBUS, signalhandler);

    parse_arguments(argc, argv);
    int idx;
    idx = num_opts * 2 + 1;
    if (pattern == NULL)
    {
        pattern = argv[idx];
        idx++;
    }
    int matched;
    int fd, file_size;
    char *inputfile;
    struct stat sb;
    while (idx < argc)
    {
        jump_val = setjmp(env_buffer);
        if (jump_val == 0)
        {
            if ((fd = open(argv[idx], O_RDONLY)) < 0)
            {
                fprintf(stderr, "Error opening %s: %s\n", argv[idx], strerror(errno));
                idx++;
                err = 1;
                continue;
            }
            if (fstat(fd, &sb) == -1)
            {
                fprintf(stderr, "Error with fstat: %s\n", strerror(errno));
                err = 1;
                continue;
            }
            file_size = sb.st_size;
            if ((inputfile = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0)) == MAP_FAILED)
            {
                perror("mmap pattern from file failed with error");
                err = 1;
                continue;
            }
            int i, num_iters;
            num_iters = file_size - pattern_size;
            current_inputfile = argv[idx];
            for (i = 0; i <= num_iters; i++)
            {
                if (memcmp(pattern, inputfile + i, pattern_size) == 0)
                {
                    matched = 1;

                    printf("%s: %d ", argv[idx], i);
                    if (cflag)
                    {
                        int j, num_chars;
                        num_chars = cval + pattern_size; // Right-side + pattern
                        int start_idx;
                        // Left side
                        if (i - cval < 0)
                        {
                            start_idx = 0;
                            num_chars += i;
                        }
                        else
                        {
                            start_idx = i - cval;
                            num_chars += cval;
                        }

                        for (j = start_idx; j < start_idx + num_chars; j++)
                        {
                            printf("%c ", inputfile[j]);
                        }
                        printf("\t");
                        for (j = start_idx; j < start_idx + num_chars; j++)
                        {
                            if (inputfile[j] < 0 || inputfile[j] > 127)
                            {
                                printf("00 ");
                            }
                            else
                            {
                                printf("%X ", inputfile[j]);
                            }
                        }
                    }
                    printf("\n");
                    break;
                }
            }
            if (close(fd) < 0)
            {
                fprintf(stderr, "error closing fd: %s\n", strerror(errno));
                err = 1;
            }
            if (munmap(inputfile, file_size) == -1)
            {
                fprintf(stderr, "Unable to munmap %s: %s\n", argv[idx], strerror(errno));
                err = 1;
            }
        }
        // Jumped
        else
        {
            if (close(fd) < 0)
            {
                fprintf(stderr, "error closing fd: %s\n", strerror(errno));
                err = 1;
            }
            if (munmap(inputfile, file_size) == -1)
            {
                fprintf(stderr, "Unable to munmap %s: %s\n", argv[idx], strerror(errno));
                err = 1;
            }
        }
        idx++;
    }
    if (err)
        return -1;
    return !matched;
}