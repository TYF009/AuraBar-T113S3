#define _POSIX_C_SOURCE 200809L
#include "file_save.h"
#include <assert.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
static int descriptors(void) {
    DIR *d=opendir("/proc/self/fd"); assert(d);
    int n=0; while(readdir(d))++n; closedir(d); return n;
}
int main(void) {
    char dir[]="/tmp/aurabar-storage-XXXXXX"; assert(mkdtemp(dir));
    setenv("AURABAR_DATA_DIR",dir,1);
    int before=descriptors();
    struct { int generation; char text[32]; } written={0}, readback={0};
    for(int i=0;i<300;++i) {
        written.generation=i; snprintf(written.text,sizeof(written.text),"record-%d",i);
        assert(file_param_write("todo.cfg",&written,sizeof(written))==0);
        assert(file_param_read("todo.cfg",&readback,sizeof(readback))==0);
        assert(!memcmp(&written,&readback,sizeof(written)));
    }
    assert(descriptors()==before);
    char path[256]; snprintf(path,sizeof(path),"%s/todo.cfg",dir);
    FILE *f=fopen(path,"wb"); assert(f); fputs("short",f); fclose(f);
    memset(&readback,0x5a,sizeof(readback));
    unsigned char saved[sizeof(readback)]; memcpy(saved,&readback,sizeof(saved));
    assert(file_param_read("todo.cfg",&readback,sizeof(readback))==-1);
    assert(!memcmp(saved,&readback,sizeof(saved)));
    assert(file_param_write("../escape",&written,sizeof(written))==-1);
    assert(file_param_write("x",&written,0)==-1);
    unlink(path); rmdir(dir);
    puts("storage: exact reads, atomic writes and descriptor lifecycle passed");
}
