/*
In this example, two children processes are created.
The first one executes the command "ls -l". Its output
is redirected to a pipe. The second one executes
"grep pipe". Its input is redirected from the
pipe the first process' output is written to. So,
the overall command lists all the *pipe* files.
The father process waits for the children to finish,
before continuing execution. This is an example of
how the "|" operator should work.

Compilation: gcc pipe-redirection.c -o pipe-redirection -Wall
*/

#include<stdio.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h> 
#include<string.h> 
#include<sys/wait.h> 

int main()
{
    int fd1[2], fd2[2];  
    int child1;
    int child2;
    pipe(fd1);
    child1 = fork();
    if(child1 == 0){
        close(fd1[0]); // not reading
        pipe(fd2);
        child2 = fork();
        char child[] = "Child\n"; 
        char grandchild[] = "Grandchild!\n";
        if(child2 == 0){
            close(fd2[0]); // not reading
            write(fd2[1], grandchild, (strlen(grandchild) + 1) * sizeof(char));
            close(fd2[1]);
            exit(0);
        }
        else{
            char gc_string[50];
            close(fd2[1]);
            write(fd1[1], child, (strlen(child) + 1) * sizeof(char));
            wait(NULL);
            read(fd2[0], gc_string, strlen(grandchild) + 1);
            //printf("gc_string: %s\n", gc_string);
            int written = write(fd1[1], gc_string, (strlen(gc_string) + 1) * sizeof(char));
            close(fd2[0]);
            close(fd1[1]);
            exit(0); 
        }
    }
    else{
        close(fd1[1]); // not writing
        char child1_str[100];
        char child2_str[100];
        read(fd1[0], child1_str, 7); 
        read(fd1[0], child2_str, 13);
        close(fd1[0]);
        printf("Child 1: %s", child1_str);
        printf("Child 2: %s", child2_str);

    }

}
