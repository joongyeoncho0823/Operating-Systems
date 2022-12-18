#include "spinlock.h"
#include "tas.h"
#include "cv.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <limits.h>
#include <stdlib.h>

void sig_handler(int sig)
{
    pid_t pid = getpid();
    // fprintf(stderr, "Got signal: %d in pid: %d\n", sig, pid);
}
void cv_init(struct cv *cv)
{
    char *addr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0);
    cv->spinlock = (struct spinlock *)(addr + sizeof(struct spinlock));
    cv->spinlock->spinlock = 0;
    int i;
    for (i = 0; i < 100; i++)
    {
        cv->waitlist[i] = 0;
    }

    sigemptyset(&(cv->mask));
    signal(SIGUSR1, sig_handler);
}

void cv_wait(struct cv *cv, struct spinlock *mutex)
{
    pid_t pid = getpid();
    spin_lock(cv->spinlock);
    int i;
    for (i = 0; i < CV_MAXPROC; i++)
    {
        if (cv->waitlist[i] == 0)
        {
            cv->waitlist[i] = pid;
            break;
        }
        if (i == 99)
        { // no more space
            fprintf(stderr, "No more space on waitlist. Exiting now...\n");
            exit(EXIT_FAILURE);
        }
    }
    spin_unlock(cv->spinlock);

    spin_unlock(mutex);
    sigsuspend(&(cv->mask));
    spin_lock(mutex);
}

int cv_broadcast(struct cv *cv)
{
    spin_lock(cv->spinlock);

    int i;
    for (i = 0; i < CV_MAXPROC; i++)
    {
        if (cv->waitlist[i] != 0)
        {
            kill(cv->waitlist[i], 10);
            cv->waitlist[i] = 0;
        }
    }
    spin_unlock(cv->spinlock);
}

int cv_signal(struct cv *cv)
{
    spin_lock(cv->spinlock);

    int i, j;

    int min_pid = INT_MAX;
    for (i = 0; i < CV_MAXPROC; i++)
    {
        if (cv->waitlist[i] != 0)
        {
            kill(cv->waitlist[i], 10);
            cv->waitlist[i] = 0;
            break;
        }
    }
    spin_unlock(cv->spinlock);
}

// int main(){
//     sigset_t set;
//     sigemptyset( &set );
//     sigaddset( &set, SIGUSR1 );
//     sigprocmask(SIG_BLOCK, &set, NULL);
//     struct cv *cv;
//     int *addr, *addr2;
//     addr =  mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0);
//     cv = (struct cv *)(addr + sizeof(struct cv));
//     struct spinlock *mutex;
//     addr2 = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0);
//     mutex = (struct spinlock *)(addr2 + sizeof(struct spinlock));
//     cv_init(cv);
//     pid_t pid2 = fork();
//     if(pid2 == 0){
//         sleep(3);
//         cv_broadcast(cv);
//         // cv_signal(cv);
//     }
//     else{
//         cv_wait(cv, mutex);
//     }
//     return 0;
// }