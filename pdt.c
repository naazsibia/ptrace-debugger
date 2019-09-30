#include "pdt.h"

AVLNode *process_tree;
DNode *dnode;


int main(int argc, char *argv[]) {
    process_tree = NULL;
    dnode = NULL;
    if (argc == 1){
        fprintf(stderr,"Too little arguments.\n");
        return 1;
    }
	pid_t child = fork();
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
	kill(getpid(), SIGSTOP);
	return execvp(args[0], args);
}

int do_trace(pid_t child){
    process_tree = insert(process_tree, child, 0);
	int status;
    pid_t newchild;
    struct user_regs_struct regs;
    waitpid(child, &status, 0);
	assert(WIFSTOPPED(status));
	assert(0 == ptrace(PTRACE_SETOPTIONS, child, NULL, SETTINGS));
	ptrace(PTRACE_SYSCALL, child, NULL, NULL);
    int children = 1;
    while (children > 0){
        newchild = waitpid(-1,&status,__WALL);
        ptrace(PTRACE_GETREGS,newchild,NULL,&regs);

        if (WSTOPEVENT(status) == PTRACE_EVENT_EXIT){
        handleExit(newchild, WEXITSTATUS(status));
            children--;
        }
        else if (WSTOPEVENT(status) == PTRACE_EVENT_FORK){
            handleFork(newchild);
            children++;
        }
        else if (regs.orig_rax == SYS_write){
        handleWrite(newchild,regs);
        } 
        else if (regs.orig_rax == SYS_read){ 
        handleRead(newchild,regs);
        }
        else if (regs.orig_rax == SYS_pipe){
            handlePipe(newchild, regs);
        }


        ptrace(PTRACE_SYSCALL, newchild, NULL, NULL);
    }
    return 0;

}
// Sahid
void handleExit(pid_t child, int exit_status){
    printf("%d exited\n", child);
    AVLNode *child_node = search(process_tree, child);
    dnode = insert_dnode(dnode, child, exit_status, child_node->open_fds, child_node->child);
    process_tree = delete_node(process_tree, child);
    printf("Preorder of new tree: ");
    pre_order(process_tree);
    printf("\n");
    return;
}

// Naaz
void handleFork(pid_t child){
    long child_forked;
    ptrace(PTRACE_GETEVENTMSG, child, NULL, &child_forked);
    printf("%d forked %ld\n", child, child_forked);
    // add child to tree -- this will copy the parents list of open fd's automatically
    process_tree = insert(process_tree, child_forked, child);
    printf("Preorder of new tree: ");
    pre_order(process_tree);
    printf("\n");
    AVLNode *child_node = search(process_tree, child_forked);
    printf("Checking copied fds: ");
    print_fd_list(child_node->open_fds);
}
//Naaz
/**
 * Switches child's debounce in avl_tree and returns
 * the current value of debounce.
**/
int switch_insyscall(pid_t child){
    AVLNode *child_node = search(process_tree, child);    
    // check if pid is in syscall
    if(!child_node->debounce){
        child_node->debounce = 1;
        return 1;
    }
    child_node->debounce = 0;
    return 0;
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
    printf("pipe(%ld) = %d\n", addr, ret_val);
    printf("piped fds: %d, %d\n", fds[0], fds[1]);
    add_fd(process_tree, child, fds[0]);
    add_fd(process_tree, child, fds[1]);
    free(fds);
}
// Ritvik
void handleWrite(pid_t child, struct user_regs_struct regs){
    AVLNode * currentNode = search(process_tree,child);
    if (currentNode->debounce == 0){
    currentNode->debounce = 1;
    char * writtenString = extractString(child,regs.rsi,regs.rdx);
    printf("%d wrote %s to File Descriptor: %lld with %lld bytes\n",child,writtenString,regs.rdi,regs.rdx); //For Development Purposes
    }else{
        currentNode->debounce = 0;
    }
}

//Ritvik
void handleRead(pid_t child, struct user_regs_struct regs){
    AVLNode * currentNode = search(process_tree,child);
    if (currentNode->debounce == 1){
    currentNode->debounce = 0;
    char * writtenString = extractString(child,regs.rsi,regs.rdx);
    printf("%d reads %s to File Descriptor: %lld with %lld bytes\n",child,writtenString,regs.rdi,regs.rdx); //For Development Purposes
    }else{
        currentNode->debounce = 1;
    }

}


int * extractArray(pid_t child, long addr, int len){
    int * array = (int *)malloc(len * sizeof(int));
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
