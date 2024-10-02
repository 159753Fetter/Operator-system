#include "ipc.h"
#include <signal.h>

void release()
{
    shmdt(buff_ptr);
    kill(getpid(),SIGSTOP);
}

int main(int argc,char *argv[])
{
    signal(SIGINT,&release);
    int rate;
    //可在在命令行第一参数指定一个进程睡眠秒数，以调解进程执行速度
    if(argv[1] != NULL)	rate = atoi(argv[1]);
    else rate = 1;	//不指定为 1 秒
    //共享内存 使用的变量
    buff_key = 101; //缓冲区任给的键值
    buff_num = 1;	//缓冲区任给的长度
    cget_key = 103; //消费者取产品指针的键值
    cget_num = 1;	//指针数
    shm_flg = IPC_CREAT | 0644; //共享内存读写权限
    //获取缓冲区使用的共享内存，buff_ptr 指向缓冲区首地址
    buff_ptr = (char *)set_shm(buff_key,buff_num,shm_flg);
    //获取消费者取产品指针，cget_ptr 指向索引地址
    cget_ptr = (int *)set_shm(cget_key,cget_num,shm_flg);
    //信号量使用的变量

    prod_key = 201;	//生产者同步信号量键值
    pmtx_key = 202;		//生产者互斥信号量键值

    glue_paper_key = 302;//有烟草的消费者B
    
    sem_flg = IPC_CREAT | 0644; //信号量操作权限

    //生产者同步信号量初值设为缓冲区最大可用量
    sem_val = buff_num;
    //获取生产者同步信号量，引用标识存 prod_sem 
    prod_sem = set_sem(prod_key,sem_val,sem_flg);

    //消费者初始无产品可取，同步信号量初值设为 0
    sem_val = 0;
    //获取消费者同步信号量，引用标识存 cons_sem 
    glue_paper_num = set_sem(glue_paper_key,sem_val,sem_flg);

    //循环执行模拟消费者不断取产品
    while(1){
        P(glue_paper_num);
        sleep(rate);
        printf("%d  %c吸烟者得到了:胶水和纸[%d]\n", getpid(), buff_ptr[*cget_ptr], *cget_ptr);
        *cget_ptr = (*cget_ptr + 1) % buff_num;
        V(prod_sem);
    }

    return EXIT_SUCCESS;
}