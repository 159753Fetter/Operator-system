#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <unistd.h>

#define BUFSZ 256
#define MAXVAL 100
#define STRSIZ 8

#define WRITERQUEST 1 // 写请求标识
#define READERQUEST 2 // 读请求标识
#define FINISHED 3    // 读写完成标识
/*信号灯控制用的共同体*/
typedef union semuns
{
    int val;
} Sem_uns;
/* 消息结构体*/
typedef struct msgbuf
{
    long mtype;
    int mid;
} Msg_buf;

// 两个信号量，一个保证互斥，一个保证相互唤醒
extern key_t costomer_key;
extern int costomer_sem;
extern key_t account_key;
extern int account_sem;
extern int sem_val;
extern int sem_flg;

//两个消息队列，一个是等待室，一个是沙发
extern int wait_quest_flg;
extern key_t wait_quest_key;
extern int wait_quest_id;

extern int wait_respond_flg;
extern key_t wait_respond_key;
extern int wait_respond_id;

extern int sofa_quest_flg;
extern key_t sofa_quest_key;
extern int sofa_quest_id;

extern int sofa_respond_flg;
extern key_t sofa_respond_key;
extern int sofa_respond_id;

int get_ipc_id(char *proc_file, key_t key);
char *set_shm(key_t shm_key, int shm_num, int shm_flag);
int set_msq(key_t msq_key, int msq_flag);
int set_sem(key_t sem_key, int sem_val, int sem_flag);
int P(int sem_id);
int V(int sem_id);
