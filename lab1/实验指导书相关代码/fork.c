#include <stdio.h>
#include <unistd.h>
int main() {
    int pid = fork();
    if(pid < 0) { //执行不成功，返回-1
        printf("fork failed\n");
    } else if (pid == 0) { //子进程执行代码
        printf("This is the child process\n");
    } else { //父进程执行代码
        printf("This is the parent process\n");
    }
     return 0;
}
