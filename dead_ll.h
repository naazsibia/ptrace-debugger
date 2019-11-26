#include <sys/types.h>
#include "avl_tree.h" // for FDNode and ProcNode

typedef struct dead_proc_node{
    int pid;
    struct timeval start_time, end_time;
    FDNode *open_fds;
    ProcNode *child;
    int num_children;
    int num_open_fds;
    int num_fds;
    int exit_status;
    int seg_fault;
    struct dead_proc_node *next; 
} DNode;


DNode* insert_dnode(DNode* node, int exit_status, AVLNode *dead);
void free_list(DNode* head);