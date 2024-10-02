#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
int main() {
    int pid = fork();
    if(pid < 0) {
        printf("fork failed\n");
    } else if(pid == 0) {
        printf("This is the child process\n");
    } else {
        wait(NULL);
        printf("This is the parent process\n");
    }
	return 0;
}
