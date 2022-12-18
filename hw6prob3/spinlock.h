#ifndef __SPINLOCK_H

struct spinlock
{
    char spinlock;
    int pid;
    int num_attempted_locks;
};

void spin_lock(struct spinlock *l);
void spin_unlock(struct spinlock *l);

#define __SPINLOCK_H
#endif