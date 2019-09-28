// for pid_t
#include <sys/types.h>
// credits: https://www.geeksforgeeks.org/avl-tree-set-2-deletion/

/**
 * LinkedList for storing the open file descriptors in 
 * a process 
**/
typedef struct fd_node{
   int fd;
   struct fd_node *next;
} FDNode;

typedef struct process_node{
    int pid;
    struct process_node *next; 
} ProcNode;

/**
 *  Binary Search Tree for storing information about a single
 * process. Each tree node contains contains a list of children. 
 * The processes are stored in order of their PIDs. Nodes with
 * smaller PIDs than the root are stored in the left subtree while
 * nodes with bigger PIDs than the root are stored in the right subtree.
 * This recursively applies to all the trees in the BST. 
 **/ 
// An AVL tree node 
typedef struct node 
{ 
    int pid; 
    int debounce; // in a syscall at the moment
    struct node *left; 
    struct node *right; 
    struct node *next; // helps store node in list
    //int num_children; // number of child processes
    ProcNode *child; // list of children
    FDNode *open_fds; // list of open fds
    int height; // important for AVL functions
} AVLNode; 



/**
 * Returns the max of pids p1 and p2
 * */
int max(pid_t p1, pid_t p2); 

/**
 * Returns the height of the tree
 **/
int height(AVLNode *n);

/**
 * Returns a copy of the list at head
**/
FDNode* copy_fd_list(FDNode *head);

/**

 * Returns a new node with the given PID
 * and NULL left and right pointers
 **/
AVLNode* new_node(int key); 

/**
 * Right rotate tree at n
 **/
AVLNode* right_rotate(AVLNode *n);

/**
 * Left rotate tree at n
**/
AVLNode *left_rotate(AVLNode *n);

/** 
 * Given a non-empty binary search tree, return the 
 * node with minimum key value found in that tree. 
 * Note that the entire tree does not need to be 
 * searched. 
**/
AVLNode * min_value_node(AVLNode* node);

/**
 * Deletes the node with pid p at node n are
 * returns the root of the modified tree
 **/
AVLNode* delete_node(AVLNode* root, pid_t p);


/**
 * Get the balance factor of node n
**/
int get_balance(AVLNode *n);

/**
 * Return the node with min value at tree rooted
 * at node n
**/
AVLNode * min_value_node(AVLNode* n);

/**
 * Insert node with pid p into AVL tree 
 * rooted at n. If Parent node p is not NULL, 
 * add the node to p's list of children
 * Return the new tree rooted at n 
**/
AVLNode* insert(AVLNode*n, pid_t pid, pid_t ppid);


/**
 * Return the node with pid p in AVLTree rooted at root
**/
AVLNode* search(AVLNode* root, pid_t p);

/**
 * Insert node with node n2 into tree at node n
**/
AVLNode* insert_node(AVLNode* n, AVLNode *n2);

/**
 * Delete node with pid p in tree rooted at
 * node n
**/
AVLNode* delete_node(AVLNode* n, pid_t p);

/**
 * Add a new node with the given pid to the list of children of the AVLNode with pid parent.
 * p is in the tree at root.
 * Returns 0 on succes, and -1 if no node with pid exists and -2 on malloc error
**/
int add_child_ppid(AVLNode* root, pid_t ppid, pid_t pid);


/**
 * Add a new node with the given pid to the list of children of parent.
 * Returns 0 on succes, and -1 if no node with pid exists and -2 on malloc error
**/
int add_child(AVLNode *parent, pid_t pid);

/**
 * Add FDnode f to the list of open file descriptors of node with pid p
 * in the tree at the given root,
 * Returns 0 on succes, and -1 on malloc error
**/
int add_fd(AVLNode* root, pid_t p,  int fd);

/**
 * Remove node with the given fd  from node n's list of open file descriptors.
 * Return 0 on success, and -1 if the node is not found
**/
int remove_fd(AVLNode* root, pid_t p, int fd); 

/**
 * Prints preorder traversal of tree at root n - helps debug
**/
void pre_order(AVLNode *n); 

/**
 * Prints the FDList (debugging purposes)
**/
void print_fd_list(FDNode *head);