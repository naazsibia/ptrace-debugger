#include<stdio.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h> 
#include<string.h> 
#include<sys/wait.h> 
int main(){
    int X = 10;
    for (int i=0; i <X;i++){
        fork();
        wait(NULL);
    }
    return 0;
}