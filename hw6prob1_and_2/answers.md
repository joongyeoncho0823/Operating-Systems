# 1
a) user-level code, single threaded, single CPU
VALID; This will not have any syncronization issues since this is running on one CPU one a single thread. All tasks will be handled sequentially through this one thread.

b) user-level code, multi-threaded, single CPU
INVALID; There are multiple threads running simultaneously, where signal handler of the interrupt is running with other threads in the CPU

c) user-level code, multi-threaded, multi CPU
INVALID; Invalid, a multi-threaded and multi-CPU approach both. For the multi-CPU approach, disabling interrupts only affect the local processor.

d) kernel-level code, single CPU
VALID; Similar to a), the protection will mask all interrupts for that one processor

e) kernel-level code, multiple CPU
INVALID; disabling interrupts on multi-CPU only affects the local processor 

# 2 
The code is vulnerable to deadlock since the locks are not strictly ordered for all tasks. In the case where both tasks are running simultaneously, taskA will lock R1 first, while taskB will simultaneously lock R2. Then, they will both check R2, R1, respectively, to see if they are locked, in which case they are by the other task. Here, nothing more can be done and leads to a deadlock. A livelock does not occur in this code since the tasks do not attempt to unwind the locked resources when both tasks are stuck at this dead end, so there is no endless loop. 