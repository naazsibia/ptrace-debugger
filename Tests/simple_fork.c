#include <stdio.h> 
#include <sys/types.h> 
#include <unistd.h> 
int main() 
{ 
  
    int child = fork();     
    if(child < 0) perror("fork");
    else if(child == 0) printf("child here\n");
    else printf("parent\n");
    return 0; 
} 