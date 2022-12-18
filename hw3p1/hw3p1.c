#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int z;

void f1()
{
    static int i = 10;
    printf("%d\n", i);
    i++;
}

main() // int is omitted
{
    int ws = -1;
    f1();
    if (fork() == 0)
    {
        f1();
    }
    f1();
    wait(&ws);
    return ws >> 8;
}
