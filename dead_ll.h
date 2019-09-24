#include <sys/types.h>

typedef struct dead_proc_node{
    int pid;
    int exit_status;
    struct dead_proc_node *next; 
} DNode;


DNode* insert_dnode(DNode* node, pid_t pid, int exit_status);
