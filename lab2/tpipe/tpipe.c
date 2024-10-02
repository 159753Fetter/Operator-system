#include <stdio.h> 
#include <unistd.h> 
#include <stdlib.h>
 #include <pthread.h>

int pipe1[2],pipe2[2];   //存放第两个无名管道标号
pthread_t thrd1,thrd2;   //存放第两个线程标识

//线程 1 执行函数，它首先向管道1写，然后从管道2读
void task1(int *num)//线程 1 执行函数原型
{
    int x=1;
    //每次循环向管道 1 的 1 端写入变量X 的值,并从
    //管道 2 的 0 端读一整数写入X 再对 X 加 1，直到X 大于 10 
    do{
        write(pipe1[1],&x,sizeof(int)); 
        read(pipe2[0],&x,sizeof(int));
        printf("thread%d read: %d\n",*num,x++); 
    }while(x<=9);
    //读写完成后,关闭管道
    close(pipe1[1]);
    close(pipe2[0]);
}
//线程 2 执行函数，它首先从管道1读，然后向管道2写
void task2(int * num)//线程 2 执行函数原型
{
    int x;
    //每次循环从管道 1 的 0 端读一个整数放入变量X 中,
    //并对 X 加 1 后写入管道 2 的 1 端，直到X 大于 10 
    do{
        read(pipe1[0],&x,sizeof(int)); 
        printf("thread2 read: %d\n",x++);
        write(pipe2[1],&x,sizeof(int));
    }while( x<=9 );
    //读写完成后,关闭管道
    close(pipe1[0]);
    close(pipe2[1]);
}

int main(int argc,char *arg[]){
    int ret;
    int num1,num2;
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
    //挂起当前线程，等待线程thrd2结束，并回收其资源 
    pthread_join(thrd2,NULL);  
    //挂起当前线程，等待线程thrd1结束，并回收其资源
    pthread_join(thrd1,NULL);  
    //思考与测试：如果去掉上述两个pthread_join的函数调用，会出现什么现象
    exit(EXIT_SUCCESS);
}