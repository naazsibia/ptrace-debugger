#include <stdio.h> 
#include <sys/types.h> 
#include <unistd.h> 
int main() 
{ 
  int i;
  int num_children = 10;
  int child;

  for(i = 0; i < num_children; i++){
      child = fork();
      if(child < 0) perror("fork");
      else if(child == 0){ 
          printf("Child number %d, ppid: %d\n", i + 1, getppid());
          return 0;
      }
  }
} 