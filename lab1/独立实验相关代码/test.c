#include "test.h"

int main(){
    int i;
    int pid1,pid2; //存放子进程号
    int status1,status2; //存放子进程返回状态
    char *argv1[] = {"/bin/ls","-a",NULL}; //子进程要缺省执行的命令
    char *argv2[] = {"/bin/ps","-a",NULL};

    signal(SIGINT, (__sighandler_t) sigcat);//注册一个本进程处理键盘中断的函数
    perror("SIGINT");  //如果系统调用signal成功执行，输出”SIGINT”，否则，

    while(1){

        pid1=fork();
        if(pid1 < 0){
            printf("Create Process child1 fail!\n");
            exit(EXIT_FAILURE); 
        }
        else if(pid1==0){//child1 process
          
            printf("I am child1 process %d ,My father process is %d\n",getpid(),getppid());
            printf("The child1 process will sleep\n");
            sleep(1);
            for(i=0; argv1[i] != NULL; i++) 
                    printf("%s ",argv1[i]);
            printf("\n");
            status1=execve(argv1[0],&argv1[0],NULL);//ls
        
        }
        else{//father process
            pid2=fork();//child2
            if(pid2<0){
                printf("Create Process child2 fail!\n");
                exit(EXIT_FAILURE);   
            }
            else if(pid2==0){//child2 process
                printf("I am child2 process %d ,My father is %d\n",getpid(),getppid());
                for(i=0; argv1[i] != NULL; i++) 
                    printf("%s ",argv1[i]);
                printf("\n");
                status2=execve(argv2[0],&argv2[0],NULL);//ls
            }
            else{//father
                printf("I am Parent process %d\n",getpid());
                printf("%d waiting for child2 done.\n",getpid());
                waitpid(pid2,&status2,0);
                printf("Child2 has done\n");
                printf("%d waiting for child1 done\n",getpid());
                waitpid(pid1,&status1,0);
                printf("Child1 has done\n");
                pause();
                sleep(3);
            }
        }
    }
    return EXIT_SUCCESS;
}