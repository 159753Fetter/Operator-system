#include "dp.h"

Sema::Sema(int id)
{
    sem_id = id;
}

Sema::~Sema() {}

/*
信号量上的down/up操作
semid:信号量数组标识符
semnum:信号量数组下标
buf:操作信号量的结构
*/

int Sema::down()
{
    struct sembuf buf;
    buf.sem_op = -1;
    buf.sem_num = 0;
    buf.sem_flg = SEM_UNDO;
    if ((semop(sem_id, &buf, 1)) < 0)
    {
        perror("down error ");
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}

int Sema::up()
{
    Sem_uns arg;
    struct sembuf buf;
    buf.sem_op = 1;
    buf.sem_num = 0;
    buf.sem_flg = SEM_UNDO;
    if ((semop(sem_id, &buf, 1)) < 0)
    {
        perror("up error ");
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}

// 用于哲学家管程的互斥进行
Lock::Lock(Sema *s)
{
    sema = s;
}

Lock::~Lock() {}

// 上锁
void Lock::close_lock()
{
    sema->down();
}

// 开锁
void Lock::open_lock()
{
    sema->up();
}

// 用于哲学家就餐问题的条件变量
Condition::Condition(char *st[], Sema *sm)
{
    state = st;
    sema = sm;
}

// 左右邻居不在就餐，条件成立，状态变为就餐
// 否则睡眠，等待条件成立
void Condition::Wait(Lock *lock, int i)
{
    if ((*state[(i + 1) % 10] != driving) &&
        (*state[(i + 3) % 10] != driving) &&
        (*state[(i + 5) % 10] != driving) &&
        (*state[(i + 7) % 10] != driving) &&
        (*state[(i + 9) % 10] != driving) &&
        (*state[i] == waiting))
    {
        *state[i] = driving; // 拿到筷子，进就餐态
    }
    else
    {
        cout << "p" << i + 1 << ":" << getpid() << " waiting ";
        if (i % 2 == 0)
            cout << "from the south to the north\n";
        else
            cout << "from the north to the south\n";
        lock->open_lock();  // 开锁
        sema->down();       // 没拿到，以饥饿态等待
        lock->close_lock(); // 上锁
    }
}

// 左右邻居不在就餐，则置其状态为就餐
// 将其从饥饿中唤醒，否则什么也不做
void Condition::Signal(int i)
{
    if ((*state[(i + 1) % 10] != driving) &&
        (*state[(i + 3) % 10] != driving) &&
        (*state[(i + 5) % 10] != driving) &&
        (*state[(i + 7) % 10] != driving) &&
        (*state[(i + 9) % 10] != driving) &&
        (*state[i] == waiting))
    {
        // 可拿到筷子，从饥饿态唤醒进用餐态
        sema->up();
        *state[i] = driving;
    }
}

/*
get_ipc_id() 从/proc/sysvipc/ 文件系统中获取IPC的id号
pfile: 对应/proc/sysvipc/ 目录中的IPC文件分别为
msg-消息队列，sem-信号量，shm-共享内存
key: 对应要获取的IPC的id号的键值
*/

int dp::get_ipc_id(char *proc_file, key_t key)
{
#define BUFSZ 256

    FILE *pf;
    int i, j;
    char line[BUFSZ], colum[BUFSZ];
    if ((pf = fopen(proc_file, "r")) == NULL)
    {
        perror("Proc file not open");
        exit(EXIT_FAILURE);
    }

    fgets(line, BUFSZ, pf);
    while (!feof(pf))
    {
        i = j = 0;

        fgets(line, BUFSZ, pf);
        while (line[i] == ' ')
            i++;
        while (line[i] != ' ')
            colum[j++] = line[i++];
        colum[j] = '\0';

        if (atoi(colum) != key)
            continue;

        j = 0;
        while (line[i] == ' ')
            i++;
        while (line[i] != ' ')
            colum[j++] = line[i++];
        colum[j] = '\0';

        i = atoi(colum);
        fclose(pf);
        return i;
    }
    fclose(pf);
    return -1;
}

/*
set_sem函数建立一个具有n个信号量的信号量
如果建立成功，返回一个信号量的标识符sem_id
输入参数：
    sem_key信号量的键值
    sem_val信号量中信号量的个数
    sem_flag信号量的存取权限
*/

int dp::set_sem(key_t sem_key, int sem_val, int sem_flg)
{
    int sem_id;
    Sem_uns sem_arg;

    // 测试由sem_key标识的信号量是否已经建立
    if ((sem_id = get_ipc_id("/proc/sysvipc/sem", sem_key)) < 0)
    {
        // semget新建一个信号量，其标号返回到sem_id
        if ((sem_id = semget(sem_key, 1, sem_flg)) < 0)
        {
            perror("semaphore create error");
            exit(EXIT_FAILURE);
        }
    }

    // 设置信号量的初值
    sem_arg.val = sem_val;
    if (semctl(sem_id, 0, SETVAL, sem_arg) < 0)
    {
        perror("semaphore set error");
        exit(EXIT_FAILURE);
    }
    return sem_id;
}

/*
set_shm函数建立一个具有n个字节的共享内存区
如果建立成功，返回一个指向该内存区首地址的指针shm_buf
输入参数：
    shm_key 共享内存的键值
    shm_val 共享内存字节的长度
    shm_flag 共享内存的存取权限
*/

char *dp::set_shm(key_t shm_key, int shm_num, int shm_flg)
{
    int i, shm_id;
    char *shm_buf;

    // 测试由shm_key标识的共享内存区是否已经建立
    if ((shm_id = get_ipc_id("/proc/sysvipc/shm", shm_key)) < 0)
    {
        // shmget新建一个长度为shm_num字节的共享内存
        if ((shm_id = shmget(shm_key, shm_num, shm_flg)) < 0)
        {
            perror("shareMemory set error");
            exit(EXIT_FAILURE);
        }
        // shmat将由shm_id标识的共享内存附加给指针shm_buf
        if ((shm_buf = (char *)shmat(shm_id, 0, 0)) < (char *)0)
        {
            perror("get shareMemory error");
            exit(EXIT_FAILURE);
        }
        for (i = 0; i < shm_num; i++)
            shm_buf[i] = 0; // 初始为0
    }
    // 共享内存区已经建立，将由shm_id标识的共享内存附加给指针shm_buf
    if ((shm_buf = (char *)shmat(shm_id, 0, 0)) < (char *)0)
    {
        perror("get shareMemory error");
        exit(EXIT_FAILURE);
    }
    return shm_buf;
}

// 哲学家就餐问题管程构造函数
dp::dp(int r)
{
    int ipc_flg = IPC_CREAT | 0644;
    int shm_key = 220;
    int shm_num = 1;
    int sem_key = 120;
    int sem_val = 0;
    int sem_id;
    Sema *sema;

    rate = r;

    // 为防止两个相邻的哲学家同时就餐，导致共有一只筷子，允许5个中只有1个
    // 在就餐，建立一个初值为1的用于锁的信号量
    if ((sem_id = set_sem(sem_key++, 1, ipc_flg)) < 0)
    {
        perror("Semaphor create error");
        exit(EXIT_FAILURE);
    }
    sema = new Sema(sem_id);
    lock = new Lock(sema);

    for (int i = 0; i < 10; i++)
    {
        // 为每一个哲学家建立一个条件变量和可共享状态
        // 初始状态都为思考
        if ((state[i] = (char *)set_shm(shm_key++, shm_num, ipc_flg)) == NULL)
        {
            perror("Share memory create error");
            exit(EXIT_FAILURE);
        }
        *state[i] = resting;
        // 为每个哲学家建立初值为0的用于条件变量的信号量
        if ((sem_id = set_sem(sem_key++, sem_val, ipc_flg)) < 0)
        {
            perror("Semaphor create error");
            exit(EXIT_FAILURE);
        }
        sema = new Sema(sem_id);
        self[i] = new Condition(state, sema);
    }
}

// 获取筷子的操作
// 如果左右邻居都在就餐，则以饥饿状态阻塞
// 否则可以进入就餐状态
void dp::pickup(int i)
{
    lock->close_lock(); // 进入管程，上锁

    *state[i] = waiting; // 饥饿状态

    self[i]->Wait(lock, i); // 测试是否能够拿到两只筷子

    cout << "p" << i + 1 << ":" << getpid() << " driving ";
    if (i % 2 == 0)
        cout << "from the south to the north\n";
    else
        cout << "from the north to the south\n";

    rate = rand() % 10 + 1;
    sleep(rate);       // 拿到，吃rate秒
    lock->open_lock(); // 离开管程，开锁
}

// 放下筷子的操作
// 状态改变为思考，如左右邻居有阻塞者则唤醒它
void dp::putdown(int i)
{
    int j;
    lock->close_lock(); // 进入管程，上锁

    *state[i] = resting; // 进休息态

    for (j = 1; j < 10; j += 2)
    {
        int t;
        t = (i + j) % 10;
        self[t]->Signal(t); // 唤醒另一方向的列车
    }

    lock->open_lock(); // 离开管程，开锁

    cout << "p" << i + 1 << ":" << getpid() << " resting ";
    if (i % 2 == 0)
        cout << "from the south to the north\n";
    else
        cout << "from the north to the south\n";

    rate = rand() % 10 + 1;
    sleep(rate); // 思考rate秒
}

dp::~dp() {}

// 哲学家就餐问题并发执行的入口

int main(int argc, char *argv[])
{
    dp *tdp;     // 哲学家就餐管程对象的指针
    int pid[10]; // 10个列车进程的进程号
    int rate;

    srand((unsigned)time(NULL));
    rate = (argc > 1) ? atoi(argv[1]) : 3;

    tdp = new dp(rate); // 建立一个哲学家就餐的管程对象

    int i;

    for (i = 0; i < 10; i++)
    {
        pid[i] = fork(); // 建立第i个列车行驶的管程对象
        if (pid[i] < 0)
        {
            perror("p1 create error");
            exit(EXIT_FAILURE);
        }
        else if (pid[i] == 0)
        { // 利用管程模拟第i个列车的行驶过程
            while (1)
            {
                tdp->pickup(i);  // 发车
                tdp->putdown(i); // 到站
            }
        }
    }

    return 0;
}