#ifndef __HW1P2_H__
#define __HW1P2_H__

struct MYSTREAM;

typedef struct MYSTREAM{
    int _file;
    int _mode; 
    int _bufsiz;
    int _currbuffsiz;
    int _curridx;
    char buff[];
} MYSTREAM;

struct MYSTREAM *myfopen(const char *pathname, int mode, int bufsiz);

struct MYSTREAM *myfdopen(int filedesc, int mode, int bufsiz);

int myfgetc(struct MYSTREAM *stream);

int myfputc(int c, struct MYSTREAM *stream);

int myfclose(struct MYSTREAM *stream);


#endif
