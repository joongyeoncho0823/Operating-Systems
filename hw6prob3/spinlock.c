#include <stdio.h>
#include <sched.h>

#include "spinlock.h"
#include "tas.h"

void spin_lock(struct spinlock *l)
{
    while (tas(&l->spinlock) != 0)
    {
        sched_yield();
    }
}
void spin_unlock(struct spinlock *l)
{
    l->spinlock = 0;
}
