#include "ipc.h"
#include <signal.h>

void release(){
    shmdt(buff_ptr);
    shmdt(pput_ptr);
    kill(getpid(),SIGSTOP);
}

int main(int argc,char *argv[])
{
    signal(SIGINT,&release);
    int rate;
    //可在在命令行第一参数指定一个进程睡眠秒数，以调解进程执行速度
    if(argv[1] != NULL)	rate = atoi(argv[1]);
    else rate = 1;	//不指定为 1 秒
    //共享内存使用的变量
    buff_key = 101;//缓冲区任给的键值
    buff_num = 1;//缓冲区任给的长度
    pput_key = 102;//生产者放产品指针的键值
    pput_num = 1; //指针数
    shm_flg = IPC_CREAT | 0644;//共享内存读写权限
    //获取缓冲区使用的共享内存，buff_ptr 指向缓冲区首地址
    buff_ptr = (char *)set_shm(buff_key,buff_num,shm_flg);
    //获取生产者放产品位置指针 pput_ptr
    pput_ptr = (int *)set_shm(pput_key,pput_num,shm_flg);


    //信号量使用的变量
    prod_key = 201;//生产者同步信号量键值
    pmtx_key = 202;//生产者互斥信号量键值

    cigarette_paper_key = 301;//有胶水的消费者A
    glue_paper_key = 302;//有烟草的消费者B
    cigarette_glue_key = 303;//有纸的消费者C

    sem_flg = IPC_CREAT | 0644;

    //生产者同步信号量初值设为缓冲区最大可用量sem_val = buff_num;
    //获取生产者同步信号量，引用标识存 prod_sem 
    sem_val = 1;
    prod_sem = set_sem(prod_key,sem_val,sem_flg);
    //消费者初始无产品可取，同步信号量初值设为 0

    sem_val = 0;
    //获取消费者同步信号量，引用标识存 cons_sem 
    cigarette_paper_num = set_sem(cigarette_paper_key, sem_val,sem_flg);
    glue_paper_num = set_sem(glue_paper_key,sem_val, sem_flg);
    cigarette_glue_num = set_sem(cigarette_glue_key,sem_val,sem_flg);

    //生产者互斥信号量初值为 1
    sem_val = 1;
    //获取生产者互斥信号量，引用标识存 pmtx_sem 
    pmtx_sem = set_sem(pmtx_key,sem_val,sem_flg);
    
    int i = 0;
    //循环执行模拟生产者不断放产品
     while(1){
        //如果缓冲区满则生产者阻塞
        P(prod_sem);
        //如果另一生产者正在放产品，本生产者阻塞
        P(pmtx_sem);
        int num = (i++)%3;
        //用写一字符的形式模拟生产者放产品，报告本进程号和放入的字符及存放的位置
        buff_ptr[*pput_ptr] = 'A'+ num;
        sleep(rate);

        if (num == 0)
            printf("供应者%d把烟草和纸放入[%d]缓存区\n", getpid(), *pput_ptr);
        else if (num == 1)
            printf("供应者%d把胶水和纸放入[%d]缓存区\n", getpid(), *pput_ptr);
        else
            printf("供应者%d把烟草和胶水放入[%d]缓存区\n", getpid(), *pput_ptr);

        //存放位置循环下移
        *pput_ptr = (*pput_ptr+1) % buff_num;

        //唤醒阻塞的生产者
        V(pmtx_sem);
        if(num == 0)
            V(cigarette_paper_num);
        else if(num == 1) 
            V(glue_paper_num);
        else 
            V(cigarette_glue_num);

    }

      return EXIT_SUCCESS;
}