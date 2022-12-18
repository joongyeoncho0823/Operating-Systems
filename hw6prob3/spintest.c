#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include "spinlock.h"
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

int N = 1024;

int main(int argc, char *argv[])
{
    int parent_pid = getpid();
    int nchild, niters;
    nchild = atoi(argv[1]);
    niters = atoi(argv[2]);

    if (argc != 3)
    {
        fprintf(stderr, "Could not parse arguments\n");
        exit(EXIT_FAILURE);
    }
    int *protected;
    int *unprotected;
    protected = mmap(NULL, N * sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0);
    unprotected = mmap(NULL, N * sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0);

    protected[0] = 0;
    unprotected[0] = 0;
    struct spinlock *lock;
    lock = (struct spinlock *)(protected + sizeof(struct spinlock));
    lock->spinlock = 0;

    int i;
    int pid;

    pid_t pids[nchild];
    for (i = 0; i < nchild; i++)
    {
        if (getpid() == parent_pid)
        {
            if ((pids[i] = fork()) < 0)
            {
                fprintf(stderr, "Error forking process %d: %s\n", i, strerror(errno));
                return EXIT_FAILURE;
            }
        }
        if (pids[i] == 0)
        {
            int j;
            for (j = 0; j < niters; j++)
            {
                unprotected[0]++;
            }
            spin_lock(lock);
            int k;
            for (k = 0; k < niters; k++)
            {
                protected[0]++;
            }
            spin_unlock(lock);
            exit(0);
        }
    }

    int m;
    for (m = 0; m < nchild; m++)
    {
        if (waitpid(pids[m], NULL, 0) < 0)
        {
            fprintf(stderr, "Error exiting child process %d\n", m);
        }
    }

    printf("Expected value from using spinlock: %d\n", nchild * niters);
    printf("Num increments for protected: %d\n", protected[0]);
    printf("Num increments for unprotected: %d\n", unprotected[0]);

    return 0;
}