#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>

#define MSGSIZE 16 
char* msg1 = "hello, world #1"; 

int main(){
    pid_t child = fork();
    if (child == 0){
        printf("Child!\n");
        return 1;
    }
    
    printf("Parent!\n");
    return 0;
}