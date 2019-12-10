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
    int fd1[2];  
    int child1;
    pipe(fd1);
    child1 = fork();
    if(child1 == 0){
        //Process B
        close(fd1[0]); // not reading
        char child[] = "Child\n"; 
        write(fd1[1], child, (strlen(child) + 1) * sizeof(char));
        int child2 = fork();
        if (child2 == 0){
            //Process C
            close(fd1[1]);
            int fd2[2];
            pipe(fd2);
            int child3 = fork();
            if (child3 == 0){
                //Process D
                close(fd2[0]);
                char child2[] = "Child\n"; 
                write(fd2[1], child2, (strlen(child2) + 1) * sizeof(char));
                close(fd2[1]);
                exit(2);
            }
            close(fd2[1]);
            char child2_str[100];
            read(fd2[0], child2_str, 7); 
            close(fd2[0]);
            printf("Child 2: %s", child2_str);
            exit(1);
        }
        close(fd1[1]);
        exit(0); 
    }
    else{
        //Process A
        close(fd1[1]); // not writing
        char child1_str[100];
        read(fd1[0], child1_str, 7); 
        close(fd1[0]);
        printf("Child 1: %s", child1_str);

    }

}
