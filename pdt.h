/* C standard library */
#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* POSIX */
#include <unistd.h>
#include <sys/user.h>
#include <sys/reg.h>
#include <sys/wait.h>
/* Linux */
#include <sys/syscall.h>
#include <sys/ptrace.h>

#include "dead_ll.h"
#include "avl_tree.h"
/* Constants */
const int long_size = sizeof(long);

/* Tracer Functions */
int do_child(int argc, char **argv);
int do_trace(pid_t child);
void handleExit(pid_t child, int exit_status);
void handleFork(pid_t child);
void handleWrite(pid_t child, struct user_regs_struct regs);
void handleRead(pid_t child, struct user_regs_struct regs);
char * extractString(pid_t child, long addr,int len);

/* Useful Definitions */
#define SETTINGS PTRACE_O_TRACEFORK | PTRACE_O_TRACESYSGOOD | PTRACE_O_TRACEEXIT
#define WSTOPEVENT(s) (s >> 16)



