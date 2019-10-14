#include "pdt.h"
#include <time.h>

AVLNode *process_tree;
DNode *dnode;


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

int do_trace(pid_t child){
    process_tree = insert(process_tree, child, 0);
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
        if(ptrace(PTRACE_GETREGS, newchild, NULL, &regs) == ESRCH){
            delete_node(process_tree, newchild);
            fprintf(stderr, "Child %d exited unexpectedly\n", newchild);
        }
        //printf("newchild: %d, syscall: %lld, type: %d, in syscall: %d\n", newchild, regs.orig_rax, status, in_syscall(newchild));
        
        if (regs.orig_rax == SYS_write && status == 34175){
            //printf("newchild: %d, syscall: %lld, type: %d, in syscall: %d\n", newchild, regs.orig_rax, status, in_syscall(newchild));
            handleWrite(newchild,regs);
        }
        
        else if (WSTOPEVENT(status) == PTRACE_EVENT_EXIT){
            if(handleExit(newchild, WEXITSTATUS(status)) == 0) children--;
        }
        else if (WSTOPEVENT(status) == PTRACE_EVENT_FORK){
            handleFork(newchild);
            children++;
        }
       
        else if (regs.orig_rax == SYS_read && status != 0){ 
            handleRead(newchild,regs);
        }
        else if (regs.orig_rax == SYS_pipe && status != 0){
            handlePipe(newchild, regs);
        }
        else if (regs.orig_rax == SYS_close && status != 0){
            //printf("handle close from %d\n", newchild);
            handleClose(newchild, regs.rdi);
        }
        // make this effiency better by making each function take a node instead of searching for the node everytime
        AVLNode *new_child_node = search(process_tree, newchild);
        if(new_child_node != NULL && !new_child_node->debounce && new_child_node->exiting){
            if(handleExit(newchild, new_child_node->exiting) == 0) children --;
        }
        /**
        sigset_t set , oldset;
        sigemptyset(&set);
        sigprocmask (SIG_SETMASK, &set, &oldset );
        nanosleep((const struct timespec[]){0,10 * 1000000}, NULL);
        sigprocmask (SIG_SETMASK, &oldset, NULL);
        **/
        
        
        if(ptrace(PTRACE_SYSCALL, newchild, NULL, NULL) == ESRCH){
            delete_node(process_tree, newchild);
            fprintf(stderr, "Child %d exited unexpectedly\n", newchild);
        }
        //printf("After: newchild: %d, syscall: %lld, type: %d, in syscall: %d\n", newchild, regs.orig_rax, status, in_syscall(newchild));
    }
    printf("Preorder of new tree: ");
    pre_order(process_tree);
    printf("\n");
    clean_tree(process_tree);
    free_list(dnode);
    return 0;


}
// Sahid
/**
 * Remove child from process tree and add it to 
 * the list of dead processes. If the child node
 * does not exist in the process tree, return -2,
 * and if the child node is currently in a syscall, return
 * -1. Else, return 0.
**/
int handleExit(pid_t child, int exit_status){
    AVLNode *child_node = search(process_tree, child);
    if(child_node == NULL){ 
        printf("Pid %d isn't in the process tree\n", child);
        return -2;
    }
    if(child_node->debounce){
        child_node->exiting = 1;
        child_node->exit_status = exit_status;
        printf("Pid %d tried to exit\n", child);
        return -1;
    }
    dnode = insert_dnode(dnode, child, exit_status, child_node->open_fds, child_node->child);
    process_tree = delete_node(process_tree, child);
    printf("Pid %d exited\n", child);
    return 0;
}

// Naaz
void handleFork(pid_t child){
    long child_forked;
    ptrace(PTRACE_GETEVENTMSG, child, NULL, &child_forked);
    printf("%d forked %ld\n", child, child_forked);
    // add child to tree -- this will copy the parents list of open fd's automatically
    process_tree = insert(process_tree, child_forked, child);
    AVLNode *child_node = search(process_tree, child_forked);
    ptrace(PTRACE_SETOPTIONS, child_forked, NULL, SETTINGS);
}

//Naaz
/**
 * Switches child's debounce in avl_tree and returns
 * the current value of debounce.
**/
int switch_insyscall(pid_t child){
    AVLNode *child_node = search(process_tree, child);    
    if(child_node == NULL) return -1;
    // check if pid is in syscall
    if(!child_node->debounce){
        child_node->debounce = 1;
        return 1;
    }
    child_node->debounce = 0;
    return 0;
}

int in_syscall(pid_t child){
    AVLNode *child_node = search(process_tree, child);
    if(child_node != NULL) return child_node->debounce;
}

void handlePipe(pid_t child, struct user_regs_struct regs){
    int* fds;
    int ret_val;
    long addr;

    int in_syscall = switch_insyscall(child);
    if(in_syscall) return; // still don't have returned value
    ret_val = (long)regs.rax;
    addr = (long)regs.rdi;
    fds = extractArray(child, addr, 8);
    add_fd(process_tree, child, fds[0]);
    add_fd(process_tree, child, fds[1]);
    printf("Pid %d piped fds %d, %d\n", child, fds[0], fds[1]);
    free(fds);
}

//Naaz
void handleClose(pid_t child, int fd){
        int ret = remove_fd(process_tree, child, fd);
        if(ret != 0) return;
            //fprintf(stderr, "FD not found\n"); -- don't need for now
        else  printf("Process %d closed fd %d\n", child, fd);
}

// Ritvik
void handleWrite(pid_t child, struct user_regs_struct regs){
    AVLNode * currentNode = search(process_tree,child);
    if(currentNode == NULL) {
        return;
    }
    if (currentNode->debounce == 0){
        char * writtenString = extractString(child,regs.rsi,regs.rdx);
        currentNode->debounce = 1;
        printf("%d wrote %s to File Descriptor: %lld with %lld bytes\n",child,writtenString,regs.rdi,regs.rdx); //For Development Purposes
        free(writtenString);
    } else{
        currentNode->debounce = 0;
        return;
    }
}

//Ritvik
void handleRead(pid_t child, struct user_regs_struct regs){
    AVLNode * currentNode = search(process_tree,child);
    if(currentNode == NULL) return;
    if (currentNode->debounce == 1){
    currentNode->debounce = 0;
    char * writtenString = extractString(child,regs.rsi,regs.rdx);
    printf("%d reads %s to File Descriptor: %lld with %lld bytes\n",child,writtenString,regs.rdi,regs.rdx); //For Development Purposes
    free(writtenString);
    }else{
        currentNode->debounce = 1;
    }

}




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
        data.val = ptrace(PTRACE_PEEKDATA,child, addr + i * 8,NULL);
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
