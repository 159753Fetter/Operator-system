#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <time.h>

using namespace std;

// 信号量控制的共用体
typedef union semuns
{
    int val;
} Sem_uns;

// 火车的3个状态（休息、等待、行驶）
enum State
{
    resting,
    waiting,
    driving
};

// 火车管程中使用的信号量
class Sema
{
public:
    Sema(int id);
    ~Sema();
    int down(); // 信号量加1
    int up();   // 信号量减1

private:
    int sem_id; // 信号量标识符
};

// 火车管程中使用的锁
class Lock
{
public:
    Lock(Sema *lock);
    ~Lock();
    void close_lock();
    void open_lock();

private:
    Sema *sema; // 锁使用的信号量
};

// 哲学家管程中使用的条件变量
class Condition
{
public:
    Condition(char *st[], Sema *sm);
    ~Condition();
    void Wait(Lock *lock, int i); // 条件变量阻塞操作
    void Signal(int i);           // 条件变量唤醒操作

private:
    Sema *sema;   // 哲学家信号量
    char **state; // 哲学家当前的状态
};

// 哲学家管程的定义
class dp
{
public:
    dp(int rate); // 管程构造函数
    ~dp();
    void pickup(int i);  // 获取筷子
    void putdown(int i); // 放下筷子

    // 建立或获取ipc信号量的一组函数的原型说明
    int get_ipc_id(char *proc_file, key_t key);
    int set_sem(key_t sem_key, int sem_val, int sem_flag);

    // 创建共享内存，放哲学家状态
    char *set_shm(key_t shm_key, int shm_num, int shm_flag);

private:
    int rate;            // 控制执行速度
    Lock *lock;          // 控制互斥进入管程的锁
    char *state[10];     // 10个车当前的状态
    Condition *self[10]; // 控制10个车状态的条件变量
};
