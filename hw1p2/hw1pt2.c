#include "hw1pt2.h"
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

struct MYSTREAM *myfopen(const char *pathname, int mode, int bufsiz){
    MYSTREAM *a = malloc(bufsiz+sizeof(char)+5*sizeof(int)+sizeof(MYSTREAM));
    if (a == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    int fd;
    if (mode == 0) {
        fd = open(pathname, O_RDONLY);
    }
    else if (mode == 1) {
       fd = open(pathname, O_WRONLY | O_CREAT, 0777);
    }
    else {
        return NULL;
    }
    if (bufsiz <= 0) {
        errno = EINVAL;
        return NULL;
    }
    a->_file = fd;
    a->_mode = mode;
    a->_bufsiz = bufsiz;
    a->_curridx = 0;
    a->_currbuffsiz = 0;
    return a; 
}

struct MYSTREAM *myfdopen(int filedesc, int mode, int bufsiz){
    MYSTREAM *a = malloc(bufsiz+sizeof(char)+5*sizeof(int)+sizeof(MYSTREAM));
    if (a == NULL) {
        return NULL;
    }
    a->_file = filedesc;
    a->_mode = mode;
    a->_bufsiz = bufsiz;
    a->_curridx = 0;
    a->_currbuffsiz = 0;
    return a;
}

int myfgetc(MYSTREAM *stream) {
    if (stream->_curridx >= stream->_currbuffsiz) {
        int readbytes = read(stream->_file, stream->buff, stream->_bufsiz);
    }
    return stream->buff[stream->_curridx++];
}

int myfputc(int c, MYSTREAM *stream) {
    stream->buff[stream->_curridx++] = c;
    if (stream->_curridx == stream->_bufsiz) {
        int numbytes = write(stream->_file, stream->buff, stream->_bufsiz);
        stream->_curridx = 0;
        return numbytes;
    }
    return c;
}

int myfclose(struct MYSTREAM *stream) {
    int fd = stream->_file;
    if (stream->_curridx < stream->_bufsiz && stream->_mode == 1) {
        write(stream->_file, stream->buff, stream->_bufsiz);
    }
    int res = close(fd);
    free(stream);
    if (res == 0) {
        return 0;
    }
    else {
        return -1; // error in close
    }   
    return 0; 
}

int main(int argc, char *argv[]){
    MYSTREAM *a = myfopen("file.txt", 0, 4096);
    MYSTREAM *b = myfopen("write_file.txt", 1, 1);
    char someChar;
    while ((someChar = myfgetc(a)) > 0) {
        myfputc(someChar, b);
    }
    int closea = myfclose(a);
    int closeb = myfclose(b);
    return 0; 
}
