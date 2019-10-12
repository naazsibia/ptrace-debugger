/* 
Notes: 
- A tracer can emulate an entire foreign operating system. This is done without any special help from the kernel beyond Ptrace.
- While a tracee can have only one tracer attached at a time, a tracer can be attached to many tracees.
- Integer Syscall args stored in: rdi, rsi, rdx, r10, r8,  r9); // r10 used instead of rcx cuz of syscalls - else it's rcx
- Mmap allocating 8 kilobytes of memory and setting the contents to 0 and returning contents to 0 and returning pointer to it. So 8000 0's. 
- Calling mmap again to copypaste entire file (libc) to the 3000 0's - directly into memory. Returning memory address of where that was mapped to. 
Libc is what program relies on - functions defined in external program. Either you need static linking - when you compile program, compiler is going 
to find external functions and build them into program itself. If you don't use static linking, it uses dynamic linking instead. Program goes to
that library and loads it into program itself. Jumps to code in memory so needs to load them to memory. (closes file after copying it
into memory)
- The file descriptor will always be the lowest fd not in use

*/

#define _POSIX_C_SOURCE 200112L

/* C standard library */
#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/* POSIX */
#include <unistd.h>
#include <sys/user.h>
#include <sys/wait.h>

/* Linux */
#include <syscall.h>
#include <sys/ptrace.h>

#define FATAL(...) \
    do { \
        fprintf(stderr, "strace: " __VA_ARGS__); \
        fputc('\n', stderr); \
        exit(EXIT_FAILURE); \
    } while (0)

int
main(int argc, char **argv)
{
    if (argc <= 1)
        FATAL("too few arguments: %d", argc);

    pid_t pid = fork();
    switch (pid) {
        case -1: /* error */
            FATAL("%s", strerror(errno));
        case 0:  /* child */
            ptrace(PTRACE_TRACEME, 0, 0, 0); // informing the kernal that it's going to be traced by its parent
            execvp(argv[1], argv + 1);
            FATAL("%s", strerror(errno)); // something went wrong with the execution oooops - error checking :P 
    }

    /* parent */
    waitpid(pid, 0, 0); // sync with PTRACE_TRACEME  --- when child returns, this will be paused 
    ptrace(PTRACE_SETOPTIONS, pid, 0, PTRACE_O_EXITKILL); // tell OS that tracee should be terminated with parent
    // real Strace may want to set other options like PTRACE_O_TRACEFORK

    for (;;) { // endless loop
        /* Enter next system call */
        /*
        The PTRACE_SYSCALL request is used in both waiting for the next system call to begin, and waiting for that system call to exit
        */ 
        if (ptrace(PTRACE_SYSCALL, pid, 0, 0) == -1) // that function waits for process to make a system call
            FATAL("%s", strerror(errno));
        if (waitpid(pid, 0, 0) == -1)
            FATAL("%s", strerror(errno));

        /* Gather system call arguments */
        struct user_regs_struct regs;
        if (ptrace(PTRACE_GETREGS, pid, 0, &regs) == -1)
            FATAL("%s", strerror(errno));
        long syscall = regs.orig_rax;

        /* Print a representation of the system call */
        fprintf(stderr, "%ld(%ld, %ld, %ld, %ld, %ld, %ld)",
                syscall,
                (long)regs.rdi, (long)regs.rsi, (long)regs.rdx,
                (long)regs.r10, (long)regs.r8,  (long)regs.r9); // r10 used instead :of rcx 

        /* Run system call and stop on exit */
        if (ptrace(PTRACE_SYSCALL, pid, 0, 0) == -1)
            FATAL("%s", strerror(errno));
        if (waitpid(pid, 0, 0) == -1)
            FATAL("%s", strerror(errno));

        /* Get system call result */
        if (ptrace(PTRACE_GETREGS, pid, 0, &regs) == -1) {
            fputs(" = ?\n", stderr);
            if (errno == ESRCH)
                exit(regs.rdi); // system call was _exit(2) or similar
            FATAL("%s", strerror(errno));
        }

        /* Print system call result */
        fprintf(stderr, " = %ld\n", (long)regs.rax);
    }
}
