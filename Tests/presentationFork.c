#include <stdio.h> 
#include <sys/types.h> 
#include <unistd.h> 
int main() 
{ 
  
    int child = fork();     
    if(child < 0) perror("fork");
    return 0; 
} 