#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

typedef struct LogNode{
    char action;
    int process;
    int fd;
    char * data;
    struct LogNode *next;
} LogNode;

typedef struct Log{
    LogNode * Head;
    LogNode * Tail;
    int size;
} LogStart;

void AddLog(LogStart *, LogNode *);
void Initialize(LogStart ** );
LogNode * RemoveLog(LogStart *);
LogNode * NewLogNode(char action,int child,int fd,char * data);