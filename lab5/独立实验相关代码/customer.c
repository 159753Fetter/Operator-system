#include "ipc.h"
int main(int argc, char *argv[])
{
    int i = 0;
    int rate;
    Msg_buf msg_arg;
    // 可在在命令行第一参数指定一个进程睡眠秒数,以调解进程执行速度
    if (argv[1] != NULL)
        rate = atoi(argv[1]);
    else
        rate = 3;

    // 联系一个请求消息队列
    wait_quest_flg = IPC_CREAT | 0644;
    wait_quest_key = 101;
    wait_quest_id = set_msq(wait_quest_key, wait_quest_flg);

    // 联系一个响应消息队列
    wait_respond_flg = IPC_CREAT | 0644;
    wait_respond_key = 102;
    wait_respond_id = set_msq(wait_respond_key, wait_respond_flg);

    // 联系一个请求消息队列
    sofa_quest_flg = IPC_CREAT | 0644;
    sofa_quest_key = 201;
    sofa_quest_id = set_msq(sofa_quest_key, sofa_quest_flg);

    // 联系一个响应消息队列
    sofa_respond_flg = IPC_CREAT | 0644;
    sofa_respond_key = 202;
    sofa_respond_id = set_msq(sofa_respond_key, sofa_respond_flg);

    // 信号量使用的变量
    costomer_key = 301; // 顾客同步信号灯键值
    account_key = 302;  // 账簿互斥信号灯键值
    sem_flg = IPC_CREAT | 0644;
    // 顾客同步信号灯初值设为 0
    sem_val = 0;
    // 获取顾客同步信号灯,引用标识存 costomer_sem
    costomer_sem = set_sem(costomer_key, sem_val, sem_flg);
    // 账簿互斥信号灯初值设为 1
    sem_val = 1;
    // 获取消费者同步信号灯,引用标识存 cons_sem
    account_sem = set_sem(account_key, sem_val, sem_flg);
    int sofa_count = 0;
    int wait_count = 0;
    while (1)//这里只设计了沙发和等待室，没有涉及到椅子
    {
        sleep(rate);
        i++;
        msg_arg.mid = i;
        if (sofa_count < 4)//说明沙发上有空位
        {
            if (wait_count != 0)//从等待室进入沙发
            {
                i--;
                // 阻塞方式接收消息
                msgrcv(wait_quest_id, &msg_arg, sizeof(msg_arg), 0, 0);//从一个消息队列里检索 (接收)消息
                // printf("mid = %d ", msg_arg.mid);
                msgsnd(wait_respond_id, &msg_arg, sizeof(msg_arg), 0);
                printf("customer %d move from the waiting room to sofa\n", msg_arg.mid);
            }
            else
            {
                printf("new customer %d sit on sofa\n", i);
            }
            sofa_quest_flg = IPC_NOWAIT;
            if (msgsnd(sofa_quest_id, &msg_arg, sizeof(msg_arg), sofa_quest_flg) >= 0)//msgsnd: 发送信息，把一条消息添加到消息队列里去
            {
            }
            sofa_count++;
        }
        else if (wait_count < 13)
        {
            printf("sofa is full, customer %d waits in the waiting room\n", i);
            wait_quest_flg = IPC_NOWAIT;
            msgsnd(wait_quest_id, &msg_arg, sizeof(msg_arg), wait_quest_flg);
            wait_count++;
        }
        else
        {
            printf("waiting room is full, customer %d don't get into the barber shop\n", i);
            msgrcv(sofa_respond_id, &msg_arg, sizeof(msg_arg), 0, 0);
            sofa_count--;
            i--;
        }
        sofa_quest_flg = IPC_NOWAIT;
        if (msgrcv(sofa_respond_id, &msg_arg, sizeof(msg_arg), 0, sofa_quest_flg) >= 0)
        {
            sofa_count--;
        }
        wait_quest_flg = IPC_NOWAIT;
        if (msgrcv(wait_respond_id, &msg_arg, sizeof(msg_arg), 0, wait_quest_flg) >= 0)
        {
            wait_count--;
        }
    }
    return 0;
}
/*
msgrcv：从一个消息队列里检索（接收）消息
msgrcv(int msqid, struct msgbuf *msgp, size_t msgsz, long msg_typ, int msgflg);
1：msgid: 由msgget函数返回的消息队列标识码
2：msg_ptr:是一个指针，指针指向准备接收的消息，
3：msg_sz发送结构体中除long int 的大小，也就是struct msgbuf中的char mtext的大小
4：msgtype:它可以实现接收优先级的简单形式
一个消息队列可以被很多进程共享，也就是多个进程都可能使用这个消息队列，那么消息如何进行区分？——mtype
mtype用来区别不同消息通常是，发送和接收的mtype数字一一对应才可以收到，当mtype==0：接收所有消息
如果mtype的值在消息队列中没有对应值且不为0，则接收将堵塞

• msgtype=0返回队列第一条信息
• msgtype>0返回队列第一条类型等于msgtype的消息　
• msgtype<0返回队列第一条类型小于等于msgtype绝对值的消息
5：msgflg:控制着队列中没有相应类型的消息可供接收时将要发生的事
• msgflg=IPC_NOWAIT，队列没有可读消息不等待，返回ENOMSG错误。
• msgflg=MSG_NOERROR，消息大小超过msgsz时被截断
• msgtype>0且msgflg=MSC_EXCEPT，接收类型不等于msgtype的第一条消息。

返回值：成功返回实际放到接收缓冲区里去的字符个数returns the number of bytes actually copied into the

*/