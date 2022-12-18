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

int isNumber(char *optarg);

int parse_options(int argc, char **argv, uid_t *uid, int *mtime, int *xflag, char **l_path);

static int
filetypeletter(int mode);

static char *lsperms(int mode);

void traverse_filesystem(char *d_name, uid_t uid, int mtime, int xflag, char *l_path, int num_recursive);

int main(int argc, char *argv[]);
