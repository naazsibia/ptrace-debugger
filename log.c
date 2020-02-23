#include "log.h"

LogNode * RemoveLog(LogStart * process_log){
    if (process_log->size == 0){
        return NULL;
    }
    LogNode * newNode = process_log->Head;
    process_log->Head = process_log->Head->next;
    process_log->size--;
    return newNode;
}
void AddLog(LogStart * process_log,LogNode * NodeToAdd){
    if (process_log->Head == NULL){
        process_log->Head = NodeToAdd;
        process_log->Tail = NodeToAdd;

    }else{
        process_log->Tail->next = NodeToAdd;
        process_log->Tail = NodeToAdd;
    }
    process_log->size++;
}

LogNode * NewLogNode(char action,int child, int fd, int inode, char * data,long bytes){
    LogNode * newNode = malloc(sizeof(LogNode));
    newNode->action = action;
    newNode->process = child;
    newNode->fd = fd;
    newNode->inode = inode;
    newNode->bytes = bytes;
    newNode->data = data;
    newNode->next = NULL;
}

void Initialize(LogStart ** process_log){
    *process_log = malloc(sizeof(LogStart));
    (*process_log)->Head = NULL;
    (*process_log)->Tail = NULL;
    (*process_log)->size = 0;
}