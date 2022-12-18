#include "fifo.h"
#include "cv.h"
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>

void fifo_init(struct fifo *f)
{
    char *cv_addr = mmap(NULL, 9182, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0);
    f->cv = (struct cv *)(cv_addr + sizeof(struct cv));
    cv_init(f->cv);
    char *sl_addr = mmap(NULL, 9182, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0);
    f->spinlock = (struct spinlock *)(sl_addr + sizeof(struct spinlock));
    f->spinlock->spinlock = 0;
    int i;
    for (i = 0; i < 1000; i++)
    {
        f->fifo[i] = 0;
    }
}

void inc_read(struct fifo *f)
{
    if (f->read_idx < 999)
    {
        f->read_idx++;
    }
    else if (f->read_idx == 999)
        f->read_idx = 0;
    else
    {
        fprintf(stderr, "Something has gone wrong\n");
        exit(EXIT_FAILURE);
    }
}

void inc_write(struct fifo *f)
{
    if (f->write_idx < 999)
    {
        f->write_idx++;
    }
    else if (f->write_idx == 999)
        f->write_idx = 0;
    else
    {
        fprintf(stderr, "Something has gone wrong\n");
        exit(EXIT_FAILURE);
    }
}

void fifo_wr(struct fifo *f, unsigned long d)
{
    spin_lock(f->spinlock);
    while (f->fifo[f->write_idx] != 0)
    {
        // printf("FIFO is full. Waiting...\n");
        cv_wait(f->cv, f->spinlock);
    }
    f->fifo[f->write_idx] = d;
    inc_write(f);
    spin_unlock(f->spinlock);
}

unsigned long fifo_rd(struct fifo *f)
{

    spin_lock(f->spinlock);

    while (f->fifo[f->read_idx] == 0)
    {
        // printf("Nothing in FIFO. Waiting...\n");
        cv_wait(f->cv, f->spinlock);
    }
    int ret = f->fifo[f->read_idx];
    f->fifo[f->read_idx] = 0;
    inc_read(f);
    spin_unlock(f->spinlock);

    return ret;
}

int main(int argc, char *argv[])
{
    pid_t parent_pid = getpid();
    // printf("ParentID: %d\n", parent_pid);
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    sigprocmask(SIG_BLOCK, &set, NULL);
    int nchild, niters;
    if (argc != 3)
    {
        fprintf(stderr, "Could not parse arguments. Exiting now...\n");
        exit(EXIT_FAILURE);
    }

    nchild = atoi(argv[1]);
    niters = atoi(argv[2]);
    size_t length = sizeof(struct fifo) + MYFIFO_BUFSIZ * sizeof(unsigned long) + sizeof(struct cv) + sizeof(struct spinlock) + 2 * sizeof(int);
    char *f_addr = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0);
    struct fifo *f;
    f = (struct fifo *)(f_addr + sizeof(struct fifo));

    fifo_init(f);
    int i;

    pid_t *pids = mmap(NULL, nchild * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    for (i = 0; i < nchild; i++)
    {
        int b = -1;
        if ((b = fork()) < 0)
        {
            fprintf(stderr, "Error forking process %d: %s\n", i, strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (b == 0)
        {
            pids[i] = getpid();
            int j;
            for (j = 0; j < niters; j++)
            {
                fifo_wr(f, pids[i]);
                spin_lock(f->spinlock);
                cv_broadcast(f->cv);
                spin_unlock(f->spinlock);
            }
            printf("Writing from pid %d COMPLETED\n", pids[i]);
            exit(0);
        }
    }
    int racks[nchild];
    int j;
    for (j = 0; j < nchild; j++)
    {
        racks[j] = 0;
    }
    int l;
    unsigned long ret;
    for (l = 0; l < nchild * niters; l++)
    {
        ret = fifo_rd(f);
        spin_lock(f->spinlock);
        cv_broadcast(f->cv);
        spin_unlock(f->spinlock);
        int k;
        for (k = 0; k < nchild; k++)
        {
            if (ret == pids[k])
            {
                racks[k]++;
                break;
            }
        }
        if (racks[k] == niters)
            printf("Reading from pid %d COMPLETED\n", pids[k]);
    }

    int m;
    for (m = 0; m < nchild; m++)
    {
        if (waitpid(pids[m], NULL, 0) < 0)
        {
            fprintf(stderr, "Error exiting child process %d\n", pids[m]);
        }
    }

    return 0;
}