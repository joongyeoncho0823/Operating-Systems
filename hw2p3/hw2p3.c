#include "hw2p3.h"

int isNumber(char *optarg)
{
  char *ptr;
  for (ptr = optarg; *ptr != '\0'; ptr++)
  {
    if (!(*ptr - '0' >= 0 && *ptr - '0' <= '9'))
    {
      return 0;
    }
  }
  return 1;
}

int parse_options(int argc, char **argv, uid_t *uid, int *mtime, int *xflag, char **l_path)
{
  int c;
  struct passwd *pwd;
  while ((c = getopt(argc, argv, "u:m:l:x")) != 1)
  {
    switch (c)
    {
    case 'u':
      if (isNumber(optarg))
      {
        *uid = atoi(optarg);
      }
      else
      {
        pwd = getpwnam(optarg);
        if (pwd == NULL)
        {
          fprintf(stderr, "ERROR: User does not exist!\n");
          return 0;
        }
        *uid = pwd->pw_uid;
      }
      break;
    case 'm':
      *mtime = atoi(optarg);
      break;
    case 'x':
      *xflag = 1;
      break;
    case 'l':
      *l_path = optarg;
      break;
    default:
      return 1;
    }
  }
  return 0;
}

// From https : // stackoverflow.com/questions/10323060/printing-file-permissions-like-ls-l-using-stat2-in-c
static int
filetypeletter(int mode)
{
  char c;

  if (S_ISREG(mode))
    c = '-';
  else if (S_ISDIR(mode))
    c = 'd';
  else if (S_ISBLK(mode))
    c = 'b';
  else if (S_ISCHR(mode))
    c = 'c';
#ifdef S_ISFIFO
  else if (S_ISFIFO(mode))
    c = 'p';
#endif /* S_ISFIFO */
#ifdef S_ISLNK
  else if (S_ISLNK(mode))
    c = 'l';
#endif /* S_ISLNK */
#ifdef S_ISSOCK
  else if (S_ISSOCK(mode))
    c = 's';
#endif /* S_ISSOCK */
#ifdef S_ISDOOR
  /* Solaris 2.6, etc. */
  else if (S_ISDOOR(mode))
    c = 'D';
#endif /* S_ISDOOR */
  else
  {
    /* Unknown type -- possibly a regular file? */
    c = '?';
  }
  return (c);
}

/*
From https://stackoverflow.com/questions/10323060/printing-file-permissions-like-ls-l-using-stat2-in-c

The pset states to not use shortcuts, but I am using the example found in the link above for this part of the assignment. (not sure if this is allowed?)
The character rwx is ordered from 0-7 in permissions. For each group of three bit, it right-shifts correspondingly the input st_mode value and performs bitwise-AND to determine the permission levels for each permission group.
*/
static char *lsperms(int mode)
{
  static const char *rwx[] = {"---", "--x", "-w-", "-wx",
                              "r--", "r-x", "rw-", "rwx"};
  static char bits[11];

  bits[0] = filetypeletter(mode);
  strcpy(&bits[1], rwx[(mode >> 6) & 7]);
  strcpy(&bits[4], rwx[(mode >> 3) & 7]);
  strcpy(&bits[7], rwx[(mode & 7)]);
  if (mode & S_ISUID)
    bits[3] = (mode & S_IXUSR) ? 's' : 'S';
  if (mode & S_ISGID)
    bits[6] = (mode & S_IXGRP) ? 's' : 'l';
  if (mode & S_ISVTX)
    bits[9] = (mode & S_IXOTH) ? 't' : 'T';
  bits[10] = '\0';
  return (bits);
}

void traverse_filesystem(char *d_name, uid_t uid, int mtime, int xflag, char *l_path, int num_recursive)
{
  struct dirent *direntp;
  struct stat info;
  DIR *folder = opendir(d_name);
  if (folder == NULL)
  {
    printf("unable to read directory");
    return;
  }
  printf("-----------------------------------");
  int lflag = strlen(l_path) != 0;
  while ((direntp = readdir(folder)) != NULL)
  {
    char path[1000] = {0};
    strcpy(path, d_name);
    strcat(strcat(path, "/"), direntp->d_name);
    stat(path, &info);

    // Name of source file of symbolic link
    char link_buf[1000];
    if (readlink(path, link_buf, sizeof link_buf) > 0)
    {
      lstat(path, &info);
    }

    // u-flag
    if (uid == 0 || uid == info.st_uid)
    {
      // Output formatting
      int i;
      for (i = 0; i < num_recursive; i++)
      {
        printf("\t");
      }
      // m-flag
      if (mtime != 0)
      {
        time_t now;
        now = time(0);
        if ((mtime > 0 && (int)(now - info.st_mtime) < mtime) || mtime < 0 && (int)(now - info.st_mtime) > -1 * mtime)
        {
          continue;
        }
      }
      // l-flag
      if (lflag == 0 || strcmp(link_buf, l_path) == 0)
      {
        if (S_ISBLK(info.st_mode) || S_ISCHR(info.st_mode))
        {
          printf("%d %d %s %d %s %s %d %d %s %s\n",
                 (int)info.st_ino, (int)info.st_blocks,
                 lsperms((int)info.st_mode), (int)info.st_nlink,
                 getpwuid(info.st_uid)->pw_name,
                 getgrgid(info.st_gid)->gr_name,
                 (int)info.st_ino, (int)info.st_dev,
                 asctime(localtime(&info.st_mtime)),
                 (S_ISLNK(info.st_mode)) ? strcat(strcat(direntp->d_name, "->"), link_buf) : direntp->d_name);
        }
        else
        {
          printf("%d %d %s %d %s %s %d %s %s\n",
                 (int)info.st_ino, (int)info.st_blocks,
                 lsperms((int)info.st_mode), (int)info.st_nlink,
                 getpwuid(info.st_uid)->pw_name,
                 getgrgid(info.st_gid)->gr_name,
                 (int)info.st_size,
                 asctime(localtime(&info.st_mtime)),
                 (S_ISLNK(info.st_mode)) ? strcat(strcat(direntp->d_name, "->"), link_buf) : direntp->d_name);
        }
      }

      // x-flag
      if (xflag && S_ISBLK(info.st_mode))
      {
        fprintf(stderr, "note: not crossing mount point at %s", path);
        continue;
      }
      // Recursively traverse directories
      if (S_ISDIR(info.st_mode))
      {
        if (strcmp(direntp->d_name, ".") != 0 && strcmp(direntp->d_name, "..") != 0)
        {
          traverse_filesystem(path, uid, mtime, xflag, l_path, num_recursive + 1);
        }
      }
    }
  }
  closedir(folder);
}

int main(int argc, char *argv[])
{
  uid_t uid = 0;
  int mtime = 0;
  int xflag = 0;
  char *l_path = "";
  if (!parse_options(argc, argv, &uid, &mtime, &xflag, &l_path))
  {
    fprintf(stderr, "Could not parse arguments.\n");
    return 0;
  }
  traverse_filesystem(argv[argc - 1], uid, mtime, xflag, l_path, 0);
  return 0;
}
