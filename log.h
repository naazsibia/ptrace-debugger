#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

typedef struct LogNode{
    char action;
    int from;
    int to;
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
LogNode * RemoveLog(LogStart *);
LogNode * NewLogNode(char action,int from,int to,int fd,char * data);