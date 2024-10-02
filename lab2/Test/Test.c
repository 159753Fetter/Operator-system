#include<stdio.h> 
#include<unistd.h> 
#include<stdlib.h>
#include<pthread.h>

int pipe1[2],pipe2[2],endx,endy;   //存放第两个无名管道标号
pthread_t thrd1,thrd2,thrd3;   //存放第两个线程标识

//线程 1 执行函数，它首先向管道1写
void task1(int *num)//线程 1 执行函数原型
{
    int x = 1;
    int f = 1;
    while (x <= endx){
        f = f * x;
        x++;
    }
    printf("Thread %d: f(x)=%d\n",*num,f);
    write(pipe1[1],&f,sizeof(int));
    //读写完成后,关闭管道
    close(pipe1[1]);
}

//线程 2 执行函数，它首先从管道1读，然后向管道2写
void task2(int * num)//线程 2 执行函数原型
{
    int y = 3;
    int f1 = 1,f2 = 1;
    //每次循环从管道 1 的 0 端读一个整数放入变量X 中,
    //并对 X 加 1 后写入管道 2 的 1 端，直到X 大于 10 
    while(y <= endy){
        int t=f2;
        f2=f1+f2;
        f1=t;
        y++;
    }
    printf("Thread %d: f(y)=%d\n",*num,f2);
    write(pipe2[1],&f2,sizeof(int));
    //读写完成后,关闭管道
    close(pipe2[1]);
}

void task3(int *num){
     int f1,f2;
     read(pipe1[0], &f1, sizeof(int));
     read(pipe2[0], &f2, sizeof(int));
     int f = f1 + f2;
     printf("Thread %d: f(x,y)=%d\n",*num,f);
     close(pipe1[0]);
     close(pipe2[0]);
}

int main(int argc,char *arg[]){
 
    while(1){
        printf("Please enter the end x, y:");
        scanf("%d %d",&endx,&endy);
        int ret;
        int num1,num2,num3;
        //使用 pipe()系统调用建立两个无名管道。建立不成功程序退出，执行终止
        if(pipe(pipe1) < 0){
            perror("pipe1 not create"); 
            exit(EXIT_FAILURE);
        }
        if(pipe(pipe2) < 0)
        { 
            perror("pipe2 not create");
            exit(EXIT_FAILURE);
        }

        //使用 pthread_create 系统调用建立两个线程。建立不成功程序退出，执行终止
        num1 = 1 ;
        ret = pthread_create(&thrd1,NULL,(void *) task1,(void *) &num1); 
        if(ret){
            perror("pthread_create: task1"); 
            exit(EXIT_FAILURE);
        }

        num2 = 2 ;
        ret = pthread_create(&thrd2,NULL,(void *) task2,(void *) &num2); 
        if(ret){
            perror("pthread_create: task2"); 
            exit(EXIT_FAILURE);
        }

        num3 = 3 ;
        ret = pthread_create(&thrd3,NULL,(void *) task3,(void *) &num3); 
        if(ret){
            perror("pthread_create: task3"); 
            exit(EXIT_FAILURE);
        }

        //挂起当前线程，等待线程thrd2结束，并回收其资源 
        pthread_join(thrd1,NULL);  
        //挂起当前线程，等待线程thrd1结束，并回收其资源
        pthread_join(thrd2,NULL);  
        //挂起当前线程，等待线程thrd1,thrd2结束，并回收其资源
        pthread_join(thrd3,NULL);
        printf("Whether will continue?\n");
        int pd = 1;
        scanf("%d",&pd);
        if(pd != 1) break;
    }
    //思考与测试：如果去掉上述两个pthread_join的函数调用，会出现什么现象
    exit(EXIT_SUCCESS);
}