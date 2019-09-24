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
        }else if (WSTOPEVENT(status) == PTRACE_EVENT_FORK){
            handleFork(newchild);
            children++;
        }else if (regs.orig_rax == SYS_write){
            handleWrite(newchild,regs);
        } else if (regs.orig_rax == SYS_read){ 
           handleRead(newchild,regs);
        }

        ptrace(PTRACE_SYSCALL, newchild, NULL, NULL);
    }
    return 0;

}


void handleExit(pid_t child, int exit_status){
    dnode = insert_dnode(dnode, child, exit_status);
    return;
}

void handleFork(pid_t child){
    long child_forked;
    ptrace(PTRACE_GETEVENTMSG, child, NULL, &child_forked);
    printf("%d forked %ld\n", child, child_forked);
    // add child to tree -- this will copy the parents list of open fd's automatically
    process_tree = insert(process_tree, child_forked, child);
    pre_order(process_tree);

}

void handleWrite(pid_t child, struct user_regs_struct regs){
}

void handleRead(pid_t child, struct user_regs_struct regs){
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
