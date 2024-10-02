#include <unistd.h>
int main() {
    char * argv[] = {"ls", "-al", "/etc/passwd", NULL};
    execvp("ls", argv);
}
