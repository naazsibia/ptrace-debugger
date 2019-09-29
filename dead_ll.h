#include <sys/types.h>
#include "avl_tree.h" // for FDNode and ProcNode
typedef struct dead_proc_node{
    int pid;
    FDNode *open_fds;
    ProcNode *child;
    int exit_status;
    struct dead_proc_node *next; 
} DNode;


DNode* insert_dnode(DNode* node, pid_t pid, int exit_status, FDNode *open_fds, ProcNode *child);
