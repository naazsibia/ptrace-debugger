/**
 *  This file contains modified code from: https://www.geeksforgeeks.org/avl-tree-set-1-insertion/. It 
 * has functions that perform standard AVL operations in addition to adding file descriptors and a linked list
 * of processes to each node. 
 *
**/

#include<stdio.h> 
#include<stdlib.h> 
#include "avl_tree.h"



/**
 * Returns the height of the tree
 **/
int height(AVLNode *n) 
{ 
    if (n == NULL) 
        return 0; 
    return n->height; 
} 
  
/**
 * Returns the max of pids p1 and p2
 * */
int max(pid_t p1, pid_t p2) 
{ 
    return (p1 > p2)? p1 : p2; 
} 
  
// Functions for AVL Tree

/**
 * Returns a new node with the given PID
 * and NULL left and right pointers
 **/
AVLNode* new_node(pid_t pid) 
{ 
    AVLNode* n = (AVLNode*) malloc(sizeof(AVLNode)); 
    if(n == NULL) {
        fprintf(stderr, "Insert failed\n");
        perror("malloc");
        return NULL; // malloc failed
    }
    n->pid   = pid; 
    gettimeofday(&(n->start_time), NULL);
    n->in_syscall = 0;
    n->exiting = 0;
    n->seg_fault = 0;
    n->num_children = 0;
    n->num_fds = 0;
    n->num_open_fds = 0;
    n->open_fds = 0;
    n->exit_status = -1;
    n->left   = NULL; 
    n->right  = NULL; 
    n->child = NULL;
    n->open_fds = NULL;
    n->height = 1;  // new node is initially added at leaf 
    return(n); 
} 

/**
 * Right rotate tree at n
 **/
AVLNode *right_rotate(AVLNode *y) 
{ 
    AVLNode *x = y->left; 
    AVLNode *T2 = x->right; 
  
    // Perform rotation 
    x->right = y; 
    y->left = T2; 
  
    // Update heights 
    y->height = max(height(y->left), height(y->right))+1; 
    x->height = max(height(x->left), height(x->right))+1; 
  
    // Return new root 
    return x; 
} 

/**
 * Left rotate tree at n
**/
AVLNode *left_rotate(AVLNode *x) 
{ 
    AVLNode *y = x->right; 
    AVLNode *T2 = y->left; 
  
    // Perform rotation 
    y->left = x; 
    x->right = T2; 
  
    //  Update heights 
    x->height = max(height(x->left), height(x->right))+1; 
    y->height = max(height(y->left), height(y->right))+1; 
  
    // Return new root 
    return y; 
} 
  

/**
 * Get the balance factor of node n
**/
int get_balance(AVLNode *n) 
{ 
    if (n == NULL) 
        return 0; 
    return height(n->left) - height(n->right); 
} 

  
/**
 * Insert node with pid p into AVL tree 
 * rooted at n. If ppid is not -1, 
 * add the node to ppid's node's list of children
 * Return the new tree rooted at n 
**/
AVLNode* insert(AVLNode*n, pid_t pid, pid_t ppid){
    AVLNode *new = new_node(pid);
    if(new == NULL){
        perror("malloc");
        return n;  // malloc failed to make new node, so return old tree
    }
    AVLNode *new_tree = insert_node(n, new); // adding node to AVLTree at n 
    if(ppid == 0) return new_tree; 
    AVLNode *parent = search(new_tree, ppid);
    if(parent == NULL){
        fprintf(stderr, "Invalid ppid\n"); // can help with debugging
        return new_tree;
    }
    if(add_child(parent, pid) == -2) perror("malloc");
    new->open_fds = copy_fd_list(parent->open_fds);
    new->num_open_fds = parent->num_open_fds;
    new->num_fds = parent->num_fds;
    return new_tree;


}


/**
 * Return the node with pid p in AVLTree rooted at root
**/
AVLNode* search(AVLNode* root, pid_t p){
    // Base Cases: root is null or key is present at root 
    if (root == NULL || root->pid == p) 
       return root; 
     
    // Key is greater than root's key 
    if (root->pid < p) 
       return search(root->right, p); 
  
    // Key is smaller than root's key 
    return search(root->left, p); 
}



/**
 * Insert node with node n2 into tree at node n
**/
AVLNode* insert_node(AVLNode* n, AVLNode *n2) 
{ 
    /* 1.  Perform the normal BST insertion */
    if (n == NULL) 
        return(n2); 
  
    if (n2->pid < n->pid) 
        n->left  = insert_node(n->left, n2); 
    else if (n2->pid > n->pid) 
        n->right = insert_node(n->right, n2); 
    else // Equal pids are not allowed in BST 
        return n; 
  
    /* 2. Update height of this ancestor node */
    n->height = 1 + max(height(n->left), 
                           height(n->right)); 
  
    /* 3. Get the balance factor of this ancestor 
          node to check whether this node became 
          unbalanced */
    int balance = get_balance(n); 
  
    // If this node becomes unbalanced, then 
    // there are 4 cases 
  
    // Left Left Case 
    if (balance > 1 && n2->pid < n->left->pid) 
        return right_rotate(n); 
  
    // Right Right Case 
    if (balance < -1 && n2->pid > n->right->pid) 
        return left_rotate(n); 
  
    // Left Right Case 
    if (balance > 1 && n2->pid > n->left->pid) 
    { 
        n->left =  left_rotate(n->left); 
        return right_rotate(n); 
    } 
  
    // Right Left Case 
    if (balance < -1 && n2->pid < n->right->pid) 
    { 
        n->right = right_rotate(n->right); 
        return left_rotate(n); 
    } 
  
    /* return the (unchanged) node pointer */
    return n; 
} 

/** 
 * Given a non-empty binary search tree, return the 
 * node with minimum key value found in that tree. 
 * Note that the entire tree does not need to be 
 * searched. 
**/
AVLNode * min_value_node(AVLNode* node) 
{ 
    AVLNode* current = node; 
  
    //loop down to find the leftmost leaf
    while (current->left != NULL) 
        current = current->left; 
  
    return current; 
} 



/**
 * Deletes the node with pid p at node n are
 * returns the root of the modified tree
 **/
AVLNode* delete_node(AVLNode* root, pid_t p) 
{ 
    // STEP 1: PERFORM STANDARD BST DELETE 
  
    if (root == NULL) 
        return root; 
  
    // If the key to be deleted is smaller than the 
    // root's key, then it lies in left subtree 
    if (p < root->pid ) 
        root->left = delete_node(root->left, p); 
  
    // If the key to be deleted is greater than the 
    // root's key, then it lies in right subtree 
    else if( p > root->pid ) 
        root->right = delete_node(root->right, p); 
  
    // if key is same as root's key, then This is 
    // the node to be deleted 
    else
    {   
        // node with only one child or no child 
        if( (root->left == NULL) || (root->right == NULL) ) 
        { 
            AVLNode *temp = root->left ? root->left : 
                                             root->right; 
  
            // No child case 
            if (temp == NULL) 
            { 
                temp = root; 
                root = NULL; 
            } 
            else{ // One child case 
             *root = *temp; // Copy the contents of 
                            // the non-empty child 
            }
            free(temp); 
        } 
        else
        { 
            // node with two children: Get the inorder 
            // successor (smallest in the right subtree) 
            AVLNode* temp = min_value_node(root->right); 
  
            // Copy the inorder successor's data to this node 
            root->pid = temp->pid; 
            root->start_time = temp->start_time;
            root->in_syscall = temp->in_syscall;
            root->exiting = temp->exiting;
            root->seg_fault = temp->seg_fault;
            root->num_children = temp->num_children;
            root->num_open_fds = temp->num_open_fds;
            root->num_fds = temp->num_fds;
            root->open_fds = temp->open_fds;
            root->child = temp->child;
            // Delete the inorder successor 
            root->right = delete_node(root->right, temp->pid); 
        } 
    } 
  
    // If the tree had only one node then return 
    if (root == NULL) 
      return root; 
  
    // STEP 2: UPDATE HEIGHT OF THE CURRENT NODE 
    root->height = 1 + max(height(root->left), 
                           height(root->right)); 
  
    // STEP 3: GET THE BALANCE FACTOR OF THIS NODE (to 
    // check whether this node became unbalanced) 
    int balance = get_balance(root); 
  
    // If this node becomes unbalanced, then there are 4 cases 
  
    // Left Left Case 
    if (balance > 1 && get_balance(root->left) >= 0) 
        return right_rotate(root); 
  
    // Left Right Case 
    if (balance > 1 && get_balance(root->left) < 0) 
    { 
        root->left =  left_rotate(root->left); 
        return right_rotate(root); 
    } 
  
    // Right Right Case 
    if (balance < -1 && get_balance(root->right) <= 0) 
        return left_rotate(root); 
  
    // Right Left Case 
    if (balance < -1 && get_balance(root->right) > 0) 
    { 
        root->right = right_rotate(root->right); 
        return left_rotate(root); 
    } 
  
    return root; 
} 


// Functions to track list of children and open-fds

/**
 * Add a new node with the given pid to the list of children of the AVLNode with pid parent.
 * p is in the tree at root.
 * Returns 0 on succes, and -1 if no node with pid exists and -2 on malloc error
**/
int add_child_ppid(AVLNode* root, pid_t ppid, pid_t pid){
    AVLNode *parent = search(root, ppid);
    if(parent == NULL) {
        fprintf(stderr, "Invalid ppid %d\n", ppid);
        return -1; 
    }
    return add_child(parent, pid);
}


/**
 * Add a new node with the given pid to the list of children of parent.
 * Returns 0 on succes, and -1 if no node with pid exists and -2 on malloc error
**/
int add_child(AVLNode *parent, pid_t pid){
    ProcNode* new_child = (ProcNode*) malloc(sizeof(ProcNode)); 
    if(new_child == NULL) return -2;
    new_child->pid = pid;
    new_child->next = NULL;
    parent->num_children++;
    ProcNode *curr = parent->child;
    if(curr == NULL) {
        parent->child = new_child;
        return 0;
    }
    while(curr->next != NULL) curr = curr->next; 
    curr->next = new_child; 
    return 0;
}


/**
 * Add a new node with the given fd to the list of open file descriptors of the AVLNode with pid p.
 * p is in the tree at root.
 * Returns 0 on succes, and -1 if no node with pid exists and -2 on malloc error
**/
int add_fd(AVLNode* root, pid_t p,  int fd, int inode, char write){
    AVLNode *parent = search(root, p);
    if(parent == NULL) return -1;
    FDNode* new_fd = (FDNode*) malloc(sizeof(FDNode)); 
    new_fd->fd = fd;
    new_fd->inode = inode;
    new_fd->next = NULL;
    new_fd->write = write;
    if(new_fd == NULL){
        perror("malloc");
        return -2;
    }
    parent->num_open_fds++;
    parent->num_fds++;
    FDNode *curr = parent->open_fds;
    if(curr == NULL){ 
        parent->open_fds = new_fd;
        return 0;
    }
    while(curr->next != NULL) {
        curr = curr->next;
    }
    curr->next = new_fd;
    return 0;
}



/**
 * Remove node with the given fd  the list of open file descriptors the 
 * process node with pid p has. p is in the tree at root.
 * Return 0 on success, and -1 if the node is not found
**/
int remove_fd(AVLNode* root, pid_t p, int inode){
    FDNode *temp = NULL;
    AVLNode *parent = search(root, p);
    if(parent == NULL) return -1;
    FDNode *curr = parent->open_fds;
    // no open fds
    if(curr == NULL) return -1;
    // head is fd
    if(curr->inode == inode) {
        parent->open_fds = curr->next;
        free(curr);
        parent->num_open_fds--;
        return 0;
    }
    while(curr->next != NULL && (curr->next)->inode != inode){
        curr = curr->next;  
    }
    if(curr->next != NULL && (curr->next)->inode == inode){
         temp = curr->next;
         curr->next = (curr->next)->next;
         free(temp);
         parent->num_open_fds--;
         return 0;
    }
    return -1; // did not find fd


} 

/**
 * Return 1 if fd is in the fd list, else return 0. 
**/
int fd_in_list(FDNode *head, int inode){
    FDNode *curr = head;
    while(curr != NULL){
        if(curr->inode == inode) return 1;
        curr = curr->next;
    }
    return 0;
}



/**
 * Returns a copy of the list at head
**/
FDNode* copy_fd_list(FDNode *head){
    if(head == NULL) return NULL;
    FDNode *new_head = (FDNode *) malloc(sizeof(FDNode));
    if(new_head == NULL){
        perror("malloc");
        return head;
    }
    new_head->fd = head->fd;
    new_head->inode = head->inode;
    new_head->write = head->write;
    new_head->next =  copy_fd_list(head->next);
    return new_head;
}

// debugging functions

/** A utility function to print preorder traversal 
* of the tree. 
* The function also prints height of every node 
**/
void pre_order(AVLNode *root) 
{   
    if(root == NULL){
        printf("(null)");
    }
    else
    { 
        printf("%d(in_syscall: %d) ", root->pid, root->in_syscall); 
        pre_order(root->left); 
        pre_order(root->right); 
    } 
} 

/**
 * Prints the FDList (debugging purposes)
**/
void print_fd_list(FDNode *head){
    if(head == NULL){
        printf("Null\n");
    }
    else{
        printf("%d (%p)->", head->fd, &head);
        print_fd_list(head->next);
    }
}


// Cleaning functions

/**
 * Frees the FD list 
**/
void free_fd_list(FDNode *head){
    FDNode *temp = NULL;
    FDNode *curr = head;
    while(curr != NULL){
        temp = curr->next;
        free(curr);
        curr = temp;
    }
}

/**
 * Frees all the AVL nodes in the given tree 
 * along with the nodes' open fds, and linked list
 * of children. 
**/
void clean_tree(AVLNode* root){
    AVLNode* curr = root;
    if(curr == NULL) return;
    printf("freeing: %d\n", root->pid);
    clean_tree(curr->left);
    clean_tree(curr->right);
    free_fd_list(curr->open_fds);
    free_the_children(curr->child);
    free(curr);   
}

/**
 * Free list of children 
**/
void free_the_children(ProcNode *head){
    ProcNode *temp = NULL;
    ProcNode *curr = head;
    while(curr != NULL){
        temp = curr->next;
        free(curr);
        curr = temp;
    }
}