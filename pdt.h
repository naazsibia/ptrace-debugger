/* C standard library */
#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>

/* POSIX */
#include <unistd.h>
#include <sys/user.h>
#include <sys/reg.h>
#include <sys/wait.h>
/* Linux */
#include <sys/syscall.h>
#include <sys/ptrace.h>

#include "dead_ll.h"
#include "log.h"
//#include "avl_tree.h"
/* Constants */
const int long_size = sizeof(long);

/* Tracer Functions */
/**
 * Make child indicate that it wants to be traced, and exec into child program.
**/
int do_child(int argc, char **argv);

/**
 * Trace syscalls that child and its successors make.
**/
int do_trace(pid_t child);

/**
* Returns 0 ff child in Process Tree and sets its segfaulted flag to 1. 
* Else returns -2.
**/
int handleSegFault(pid_t child);

/**
 * Remove child from process tree and add it to 
 * the list of dead processes. If the child node
 * does not exist in the process tree, return -2,
 * and if the child node is currently in a syscall, return
 * -1. Else, return 0.
**/
int handleExit(pid_t child);

/**
 * Extract PID of child forked, and insert it into the
 * Process tree, its parents list of children, and copy
 * its parent's list of open fds.
 * 
**/
void handleFork(pid_t child);

/**
 * Add the fds created by pipe() to child's list of open
 * fds. 
**/
void handlePipe(pid_t child, struct user_regs_struct regs);

/**
 * Remove the closed fds from child's list of open fds. 
**/
void handleClose(pid_t child, int fd);

/**
 * Extracts data from memory offset in registers for the write call and 
 * saves it as a log node 
**/
void handleWrite(pid_t child, struct user_regs_struct regs);

/**
 * Extracts data from memory offset in registers for the read call and 
 * saves it as a log node 
**/
void handleRead(pid_t child, struct user_regs_struct regs);

/* Other helpers */
/**
 * Switches child's in_syscall in avl_tree and returns
 * the current value of in_syscall.
**/
int switch_insyscall(pid_t child);

/**
 * Return 1 if child is in syscall, else return 0.
**/
int in_syscall(pid_t child);

/**
 * Return -1 if stat is unsuccessful in locating the fd, 0 if file is not a FIFO
 * and non-zero value otherwise
**/
int is_pipe(int child, int fd);

/**
 * Get inode for a given fd. Return -1 if stat is unsuccessful in locating the fd
**/
int get_inode(int child, int fd);


/**
 * Extract len ints given a memory address addr.
**/
int * extractArray(pid_t child, long addr, int len);

/**
 * Return a string of len characters at a given address given process pid
**/
char * extractString(pid_t child, long addr,int len);

/**
 * Writes extracted data to csv with name filename
**/
int csvWrite(char * filename);

/**
 * Writes log data for node to csv file
**/
void writeLogData(LogNode *node, FILE *file);

/**
 * Writes process tree data to csv file
**/
void writeNodeData(DNode *node, FILE* file);

/* Useful Definitions */
#define SETTINGS PTRACE_O_TRACEFORK | PTRACE_O_TRACESYSGOOD | PTRACE_O_TRACEEXIT
#define WSTOPEVENT(s) (s >> 16)
#define MAXSIZE 500


