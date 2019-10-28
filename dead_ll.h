#include <sys/types.h>
#include "avl_tree.h" // for FDNode and ProcNode

typedef struct dead_proc_node{
    int pid;
    FDNode *open_fds;
    ProcNode *child;
    int num_children;
    int num_open_fds;
    int exit_status;
    struct dead_proc_node *next; 
} DNode;


DNode* insert_dnode(DNode* node, int exit_status, AVLNode *dead);
void free_list(DNode* head);