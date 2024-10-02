#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h>
#include <sys/types.h> 
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <sys/sem.h> 
#include <sys/msg.h>

#define BUFSZ	256
//建立或获取 ipc 的一组函数的原型说明
int get_ipc_id(char *proc_file,key_t key);

char *set_shm(key_t shm_key,int shm_num,int shm_flag); 
int set_msq(key_t msq_key,int msq_flag);
int set_sem(key_t sem_key,int sem_val,int sem_flag); 
int P(int sem_id);

int V(int sem_id);

/*信号量控制用的共同体*/ 
typedef union semuns {
      int val;
} Sem_uns;

/* 消 息 结 构 体 */ 
typedef struct msgbuf {
      long mtype; 
      char mtext[1];
} Msg_buf;

//生产消费者共享缓冲区即其有关的变量
extern key_t buff_key;
extern int buff_num;
extern char *buff_ptr;

//生产者放产品位置的共享指针
extern key_t pput_key;
extern int pput_num; 
extern int *pput_ptr;

//消费者取产品位置的共享指针
extern key_t cget_key;
extern int cget_num;
extern int *cget_ptr;

//生产者有关的信号量
extern key_t prod_key;
extern key_t pmtx_key; 
extern int prod_sem; 
extern int pmtx_sem;

//消费者有关的信号量
extern key_t cons_key;
extern key_t cmtx_key; 
extern int cons_sem; 
extern int cmtx_sem;

extern int sem_val; 
extern int sem_flg; 
extern int shm_flg;