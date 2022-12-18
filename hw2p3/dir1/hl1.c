#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <pwd.h>
#include <ctype.h>
#include <string.h>
#include <grp.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

int isNumber(char **optarg)
{
    char *ptr;
    for (ptr = *optarg; *ptr != '\0'; ptr++)
    {
        if (!(*ptr - '0' >= 0 && *ptr - '0' <= '9'))
        {
            return 0;
        }
    }
    return 1;
}

int main()
{
    char str[] = "1123";
    char *string = str;
    int x = isNumber(&string);
    printf("%d", x);
    return 0;
}