#include "pdt.h"
#include <time.h>

AVLNode *process_tree;
DNode *dnode;
LogStart * process_log = NULL;
int dead_children = 0;

int main(int argc, char *argv[]) {
    process_tree = NULL;
    dnode = NULL;
    if (argc == 1){
        fprintf(stderr,"[usage] ./pdt ./{exec_name}\n");
        return 1;
    }
	pid_t child = fork();
    if(child < 0){
        perror("fork");
        return -1;
    }
	if(child == 0) {
		return do_child(argc - 1, argv + 1);
	} else {
		return do_trace(child);
	}

	return 0;
}

/**
 * Make child indicate that it wants to be traced, and exec into child program.
**/
int do_child(int argc, char **argv) {
	char *args [argc + 1];
	int i;
	for (i = 0; i < argc; i++)
		args[i] = argv[i];
	args[argc] = NULL;
    assert( 0 == ptrace(PTRACE_TRACEME));
	if(kill(getpid(), SIGSTOP) < 0) perror("kill");
	return execvp(args[0], args);
}

/**
 * Trace syscalls that child and its successors make.
**/
int do_trace(pid_t child){
    process_tree = insert(process_tree, child, 0);
    Initialize(&process_log);
    if(process_tree == NULL){
        fprintf(stderr, "Insert failed\n");
        return -1;
    }
	int status;
    pid_t newchild;
    struct user_regs_struct regs;
    if(waitpid(child, &status, 0) < 0) perror("waitpid");
	assert(WIFSTOPPED(status));
	assert(0 == ptrace(PTRACE_SETOPTIONS, child, NULL, SETTINGS));
	ptrace(PTRACE_SYSCALL, child, NULL, NULL);
    int children = 1;
    

    while (children > 0){
        newchild = waitpid(-1, &status, __WALL);
        if(newchild < 0){
              perror("waitpid"); // if there are no child processes, will not get syscalls
              break;
        }
        if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGSEGV){
            if (handleSegFault(newchild) == 0) children --;
        }
        if(ptrace(PTRACE_GETREGS, newchild, NULL, &regs) == ESRCH){
            delete_node(process_tree, newchild);
            fprintf(stderr, "Child %d exited unexpectedly\n", newchild);
        }
           
        if(WIFSTOPPED(status) && (WSTOPSIG(status) == (SIGTRAP|0x80))){
            if(regs.orig_rax == SYS_write){
                handleWrite(newchild,regs);
            }
            else if (regs.orig_rax == SYS_read){ 
                handleRead(newchild,regs);
            }
            else if (regs.orig_rax == SYS_pipe){
                handlePipe(newchild, regs);
            }
            else if (regs.orig_rax == SYS_close){
                handleClose(newchild, regs.rdi);
            } 
        }
        else if (WIFSTOPPED(status)){
            if (status>>8 == (SIGTRAP | (PTRACE_EVENT_EXIT<<8))){
                if(handleExit(newchild) == 0) children--;
            }
            else if (WSTOPEVENT(status) == PTRACE_EVENT_FORK){
                handleFork(newchild);
                children++;
            }
        }

        AVLNode *new_child_node = search(process_tree, newchild);
        // if child has exited and isn't in a syscall, remove it from the process tree
        if(new_child_node != NULL && !new_child_node->in_syscall && new_child_node->exiting){
            if(handleExit(newchild) == 0) children --;
        }
        
        // child exited unexpectedly, can't trace it anymore
        if(ptrace(PTRACE_SYSCALL, newchild, NULL, NULL) == ESRCH){
            delete_node(process_tree, newchild);
            fprintf(stderr, "Child %d exited unexpectedly\n", newchild);
        }
    }
    
    printf("Preorder of new tree: ");
    pre_order(process_tree); 
    printf("\n");
    csvWrite("test.csv");
    
    //REMOVE THIS LATER WHEN WRIITNG CSV MAKE SURE TO FREE NODES
    while(process_log->size != 0){
        LogNode * node = RemoveLog(process_log);
        free(node->data);
        free(node);
    }
    free(process_log);
    clean_tree(process_tree);
    free_list(dnode);
    return 0;


}
// Sahid

int handleSegFault(pid_t child){
    AVLNode * child_node = search(process_tree, child);
    if(child_node == NULL){ 
       // printf("Pid %d isn't in the process tree\n", child);
        return -2;
    }
    child_node->seg_fault = 1;
    dnode = insert_dnode(dnode, -1, child_node);
    process_tree = delete_node(process_tree, child);
    dead_children++;
    printf("Pid %d exited with seg fault\n", child);
    return 0;
}
/**
 * Remove child from process tree and add it to 
 * the list of dead processes. If the child node
 * does not exist in the process tree, return -2,
 * and if the child node is currently in a syscall, return
 * -1. Else, return 0.
**/
int handleExit(pid_t child){
    AVLNode * child_node = search(process_tree, child);
    if(child_node == NULL){ 
        printf("Pid %d isn't in the process tree\n", child);
        return -2;
    }
    long exit;
    ptrace(PTRACE_GETEVENTMSG,child,NULL,&exit);
    gettimeofday(&(child_node->end_time), NULL);
    if(child_node->in_syscall){
        child_node->exiting = 1;
        child_node->exit_status = WEXITSTATUS(exit);
        printf("Pid %d tried to exit\n", child);
        return -1;
    }
    // edit to just pass node in
    dnode = insert_dnode(dnode, WEXITSTATUS(exit), child_node);
    process_tree = delete_node(process_tree, child);
    dead_children++;
    printf("Pid %d exited\n", child);
    return 0;
}

// Naaz
/**
 * Extract PID of child forked, and insert it into the
 * Process tree, its parents list of children, and copy
 * its parent's list of open fds.
 * 
**/
void handleFork(pid_t child){
    long child_forked;
    ptrace(PTRACE_GETEVENTMSG, child, NULL, &child_forked);
    printf("%d forked %ld\n", child, child_forked);
    // add child to tree -- this will copy the parents list of open fd's automatically
    process_tree = insert(process_tree, child_forked, child);
    AVLNode *child_node = search(process_tree, child_forked);
    ptrace(PTRACE_SETOPTIONS, child_forked, NULL, SETTINGS);
}

/**
 * Add the fds created by pipe() to child's list of open
 * fds. 
**/
void handlePipe(pid_t child, struct user_regs_struct regs){
    int* fds;
    int ret_val;
    long addr;

    int in_syscall = switch_insyscall(child);
    if(in_syscall) return; // still don't have returned value
    ret_val = (long)regs.rax;
    addr = (long)regs.rdi;
    fds = extractArray(child, addr, 8);
    printf("pid: %d, fd1: %d, fd2: %d, inode1: %d, inode2: %d\n", child, fds[0], fds[1], get_inode(child,fds[0]), get_inode(child,fds[1]));
    add_fd(process_tree, child, get_inode(child,fds[0]));
    add_fd(process_tree, child, get_inode(child,fds[1]));
    printf("Pid %d piped fds %d, %d\n", child, fds[0], fds[1]);
    free(fds);
}

/**
 * Remove the closed fds from child's list of open fds. 
**/void handleClose(pid_t child, int fd){
        int ret = remove_fd(process_tree, child, get_inode(child,fd));
       
       /* if(ret != 0){
            fprintf(stderr, "FD %d %d not found\n", fd, get_inode(child,fd)); //-- don't need for now
        }
        else  printf("Process %d closed fd %d, %d\n", child, fd, get_inode(child,fd));*/
}

// Ritvik
void handleWrite(pid_t child, struct user_regs_struct regs){
    AVLNode * currentNode = search(process_tree,child);
    if(currentNode == NULL) {
        return;
    }
    if (fd_in_list(currentNode->open_fds,get_inode(child,regs.rdi)) == 0) return;
    if (currentNode->in_syscall == 0){
        char * writtenString = extractString(child,regs.rsi,regs.rdx);
        currentNode->in_syscall = 1;
        LogNode * node = NewLogNode('W',child,get_inode(child,(int) regs.rdi),writtenString,regs.rdx);
        AddLog(process_log,node);
        printf("%d wrote %s to File Descriptor: %lld with %lld bytes\n",child,writtenString,regs.rdi,regs.rdx); //For Development Purposes
    } else{
        currentNode->in_syscall = 0;
        return;
    }
}

//Ritvik
void handleRead(pid_t child, struct user_regs_struct regs){
    AVLNode * currentNode = search(process_tree,child);
    if(currentNode == NULL ) return;
    int fd = (int) regs.rdi;
    if (fd_in_list(currentNode->open_fds,get_inode(child,fd)) == 0) return;
    if (currentNode->in_syscall == 1){
    currentNode->in_syscall = 0;
    char * writtenString = extractString(child,regs.rsi,regs.rdx);
    LogNode * node = NewLogNode('R', child, get_inode(child,(int) regs.rdi), writtenString,regs.rdx);
    AddLog(process_log,node);
    printf("%d reads %s to File Descriptor: %lld with %lld bytes\n", child, writtenString, regs.rdi, regs.rdx); //For Development Purposes
    }else{
        currentNode->in_syscall = 1;
    }
}


//Naaz
/**
 * Switches child's in_syscall in avl_tree and returns
 * the current value of in_syscall.
**/
int switch_insyscall(pid_t child){
    AVLNode *child_node = search(process_tree, child);    
    if(child_node == NULL) return -1;
    // check if pid is in syscall
    if(!child_node->in_syscall){
        child_node->in_syscall = 1;
        return 1;
    }
    child_node->in_syscall = 0;
    return 0;
}

/**
 * Return 1 if child is in syscall, else return 0.
**/
int in_syscall(pid_t child){
    AVLNode *child_node = search(process_tree, child);
    if(child_node != NULL) return child_node->in_syscall;
}

int csvWrite(char * filename){
    FILE *file;
    if((file = fopen(filename, "w+")) == NULL) return -1;
    DNode *curr = dnode;
    fprintf(file, "%d, %d\n", dead_children, process_log->size);
    while(curr != NULL){
        writeNodeData(curr, file);
        curr = curr->next;
    }
    LogNode *start = process_log->Head;
    while(start != NULL){
        writeLogData(start, file);
        start = start->next;
    }
    return 0;

}

void writeLogData(LogNode *node, FILE *file){
    fprintf(file, "%c, %d, %d, %s\n", node->action, node->process, node->fd, node->data);
} 




void writeNodeData(DNode *node, FILE *file){
    // format `pid, exit_status, num children, num_openfds` 
    
    fprintf(file, "%d, %ld.%ld, %ld.%ld, %d, %d, %d, %d, ", node->pid, (long) node->start_time.tv_sec,  (long) node->start_time.tv_usec, (long) node->end_time.tv_sec,(long) node->end_time.tv_usec , node->exit_status, node->seg_fault, node->num_children, node->num_open_fds);

    ProcNode *curr = node->child;
    while(curr != NULL){
        fprintf(file, "%d, ", curr->pid);
        curr = curr->next;
    }
    FDNode *curr2 = node->open_fds;
    while(curr2 != NULL){
        fprintf(file, "%d, ", curr2->fd);
        curr2 = curr2->next;
    }
    fprintf(file, "\n");
}


int is_pipe(int child,int fd){
    char buffer[80];
    sprintf(buffer,"/proc/%d/fd/%d",child,fd);
    struct stat info;
    if (stat(buffer,&info) == -1) return -1;
    return S_ISFIFO(info.st_mode);
}

int get_inode(int child, int fd){
    char buffer[80];
    sprintf(buffer,"/proc/%d/fd/%d",child,fd);
    struct stat info;
    if (stat(buffer,&info) == -1) return -1;
    return info.st_ino;
}


/**
 * Extract len ints given a memory address addr.
**/
int * extractArray(pid_t child, long addr, int len){
    int * array = (int *)malloc(len * sizeof(int));
    if(array == NULL) perror("malloc");
    int *laddr;
    int i, j;
    union u {
        long val;
        int int_array[long_size];
        }data;
    i = 0;
    j = len / long_size;
    laddr = array;
    while(i < j){
        data.val = ptrace(PTRACE_PEEKDATA, child, addr + i * 8, NULL);
        memcpy(laddr, data.int_array, long_size);
        ++i;
        laddr += long_size;
    }
    j = len % long_size;
    if(j != 0) {
        data.val = ptrace(PTRACE_PEEKDATA,child, addr + i * 8,NULL);
        memcpy(laddr, data.int_array, j);
    }
    return array;
}

char * extractString(pid_t child, long addr, int len) { 
    char * str = (char *)malloc((len+1) * sizeof(char));
    if(str == NULL) perror("malloc");
    char *laddr;
    int i, j;
    union u {
        long val;
        char chars[long_size];
        }data;
    i = 0;
    j = len / long_size;
    laddr = str;
    while(i < j) {
        data.val = ptrace(PTRACE_PEEKDATA, child, addr + i * 8,NULL);
        memcpy(laddr, data.chars, long_size);
        ++i;
        laddr += long_size;
    }
    j = len % long_size;
    if(j != 0) {
        data.val = ptrace(PTRACE_PEEKDATA,child, addr + i * 8,NULL);
        memcpy(laddr, data.chars, j);
    }
    str[len] = '\0';
    return str;
}
