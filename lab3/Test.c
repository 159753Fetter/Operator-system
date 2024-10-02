#include <sys/types.h>
#include <wait.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>
#include <malloc.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <pwd.h>
#include <curses.h>

/* 用不同的色彩显示用户名与工作目录*/
#define UserNameColor "\033[1m\033[32m"
#define PathColor "\033[1m\033[34m"
#define ColorEnd "\033[0m"

#define MAX_LINE 127  /*限定用户输入的命令行长度不超过127字符*/
pid_t parentpid;//用来记录父进程的pid，因为我们要终止子进程，而不是终止父进程
pid_t childpid;//用来记录子进程的pid
int fd;//文件标识符
int fildes[2];//管道
int status; //子进程的退出状态
int background=0;//如果命令行的最后字符是&，则转入后台运行，并且background=1 

typedef void (*sighandler_t)(int);
void sigcat(){
    if(getpid() == parentpid){ //父进程自定义信号SIGINT的处理程序，当按下ctrl-c后，不终止父进程(Shell)，只终止其子程序。
        kill(childpid, SIGINT); //设置其它信号，当父进程(shell)收到该信号后，结束执行，退出
    }
}

//将命令分割
int get_order(char* arvs, char** argv){

    int argc = 0;
    int flag = 0;
    int Len = strlen(arvs) - 1;
    for (int i = 0; i < Len; i++){
        if (arvs[i] != ' ' && flag == 0){//判断是否遇到空格，如果遇到了的话就分开
            argv[argc++] = arvs + i;
            flag = 1;
        }
        else if (arvs[i] == ' '){
            flag = 0;
            arvs[i] = 0;
        }
    }
    argv[argc] = NULL;
    return argc; // 返回该命令的字符串数量
}

char *getUserName(){ //get user name
    struct passwd *pwd = getpwuid(getuid());
    return pwd->pw_name;
}
char *getUserHomeDirectory(){ //get user home directory
    struct passwd *pwd = getpwuid(getuid());
    return pwd->pw_dir;
}

char* LRTrim(char *str){
   while (*str==' ') 
     str++;
   int len=strlen(str)-1;
   while (str[len]==' ')
     len--;
   str[len+1]='\0';
   return str;
}

/********************************************
int Exec_Cd(char *Command)
命令cd是一个内置命令，不能利用exec执行，需要
单独处理, cd, cd .., cd ~, cd 路径名 等
********************************************/
void Exec_Cd(char *Command){
   char NewPath[MAX_LINE];
   char *str=LRTrim(Command);
   if (strstr(str,"..")!=NULL)
   {
      char *CurrentPath=getcwd(NULL,0);
      char *p=strrchr(CurrentPath,'/');
      if (p==NULL)
         return;
      int n=strlen(CurrentPath)-strlen(p);
      strncpy(NewPath,CurrentPath,n);
      NewPath[n]='\0';
   }
   else if ((strstr(str,"~")!=NULL) || (strcmp(str,"cd")==0))
   {
      strcpy(NewPath,getUserHomeDirectory());
   }
   else
   {
      str=str+2;  //get the path followed "cd"
      strcpy(NewPath,LRTrim(str));
   }
   if (access(NewPath,F_OK)==0)
      chdir(NewPath);
}

/**********************************************************
char *SubStrReplace(char *Str, char *SubStr, char *Replace)
* 将字符串Str中的所有子串SubStr替换为Replace
**********************************************************/
char *SubStrReplace(char *Str, char *SubStr, char *Replace){
    char *ret = strstr(Str, SubStr);
    if (ret == NULL) // SubStr is not in Str
        return Str;

    char NewStr[256];
    int n = strlen(Str) - strlen(ret);

    strncpy(NewStr, Str, n);
    NewStr[n] = '\0';

    strcat(NewStr, Replace);

    for (int i = 1; i <= strlen(SubStr); i++)
        ret = ret + 1;

    strcat(NewStr, ret);

    return SubStrReplace(NewStr, SubStr, Replace);
}

//这是执行函数
void solution(char* arvs){
    
    int output_redirection = 0; // 输出重定向
    int input_redirection = 0;  // 输入重定向
    char* outfile;
    char* inputfile;

    char* command[MAX_LINE]; //用来将命令拆分后进行存储

    char* str;
    str = (char *)malloc(sizeof(char)*strlen (arvs));
    for(int i = 0;i<strlen(arvs);i++)
        str[i]=arvs[i];

    int argc = get_order(arvs,command);
    int targc = argc;//用来记录下在哪有重定向，因为重定向之前的才是命令

    for (int i = 0; i < argc; i++){
        if (strcmp(command[i], "<") == 0){ //strcmp函数用来进行字符串比较，相等返回0，如果拆分后的命令是<
             if (i + 1 == argc){//标准输入重定向到文件
                printf("file not exist\n");
                exit(1);
            }
            else{//说明文件存在，那么将输入重定向到文件
                inputfile = command[++i];
                input_redirection = 1;
                if (targc == argc)
                    targc = i - 1;
            }
        }

        if (strcmp(command[i], "<<") == 0){
            if (i + 1 == argc){// 从标准输入中读入，直到遇到“分界符”停止。
                printf("file not exist\n");
                exit(1);
            }
            else{
                inputfile = command[++i];
                input_redirection = 2;
                if (targc == argc)
                    targc = i - 1;
            }
        }

        if (strcmp(command[i], ">") == 0){
            if (i + 1 == argc){//标准输出重定向到文件
                printf("file not exist\n");
                exit(1);
            }
            else{//说明文件存在，那么将输入重定向到文件
                outfile = command[++i];
                output_redirection = 1;
                if (targc == argc)
                    targc = i - 1;
            }
        }

        if (strcmp(command[i], "2>") == 0){//将错误输出重定向到文件中,清除原有文件中的数据,这里的2表示文件描述符2，即标准错误输出。
            if (i + 1 == argc){
                printf("file not exist\n");
                exit(1);
            }
            else{
                outfile = command[++i];
                output_redirection = 2;
                if (targc == argc)
                    targc = i - 1;
            }
        }

        if (strcmp(command[i], ">>") == 0){
            if (i + 1 == argc){//将标准输出重定向到文件中（在原有的内容后追加），注意这个跟>的不同之处在于一个是清空原来所有的，一个是在后面加
                printf("file not exist\n");
                exit(1);
            }
            else{
                outfile = command[++i];
                output_redirection = 3;
                if (targc == argc)
                    targc = i - 1;
            }
        }

        if (strcmp(command[i], "2>>") == 0){
            if (i + 1 == argc){//将错误输出重定向到文件中（在原有内容后面追加）
                printf("file not exist\n");
                exit(1);
            }
            else{
                outfile = command[++i];
                output_redirection = 4;
                if (targc == argc)
                    targc = i - 1;
            }
        }

        if (strcmp(command[i], "&>>") == 0){
            if (i + 1 == argc){//标准输出和错误输出都写入文件（在原有内容后追加）
                printf("file not exist\n");
                exit(1);
            }
            else{
                outfile = command[++i];
                output_redirection = 5;
                if (targc == argc)
                    targc = i - 1;
            }
        }
    }

    childpid = fork();//创建一个子进程
    if(childpid < 0){
        printf("create fork failed.\n");
        exit(1);
    }

    else if(childpid == 0){//处理执行命令行输入的命令
        signal(SIGINT, sigcat); // 定义中断函数
        //设置信号SIGINT的处理方式默认SIG_DFL，按下ctrl-c后，终止子进程,命令行中是否有I/O重定向，即双方>,<等符号

        if (input_redirection == 1){ // 输入重定向<,重定向文件inputfile到标准输入
            fd = open(inputfile, 777);
            close(0); // close(stdin)
            dup(fd);//dup()将一个文件描述符复制到该用户文件描述符表的第一个空的表项中
            close(fd);
        }                         
        if (input_redirection == 2){  //因为是2标准输入中读入，所以不考虑打开文件
            fd = open(inputfile, 777);
            close(0); // close(stdin)
            dup(fd);
            close(fd);
        }
 
        if (output_redirection == 1){ // 输出重定向>,重定向标准输出到文件outfile
            fd = open(outfile, O_CREAT | O_WRONLY, 0644);
            close(1); // close(stdout)
            dup(fd);
            close(fd);
        }                            

        if (output_redirection == 2){ // 输出重定向2>,重定向错误到文件outfile
            fd = open(outfile, O_CREAT | O_WRONLY, 0644);
            close(1); // close(stdout)
            dup(fd);
            close(fd);
        }                            

        if (output_redirection == 3){ // 输出重定向>>,重定向标准输出到文件outfile后加方式    
            fd = fopen(outfile, "a+");
            close(1); // close(stdout)
            dup(fd);
            close(fd);
        }                          

        if (output_redirection == 4){ // 输出重定向2>>,重定向错误输出到文件outfile后加方式
            fd = fopen(outfile, "a+");
            close(1); // close(stdout)
            dup(fd);
            close(fd);
        }                           

        if (output_redirection == 5){ // 输出重定向&>>,重定向标准输出和错误输出到文件outfile
            fd = fopen(outfile, "a+");
            close(1); // close(stdout)
            dup(fd);
            close(fd);
        }

        // 重定向之前的内容为命令，才有意义
        char* temp[100];
        for (int i = 0; i < targc; i++)
            temp[i] = command[i];
        temp[targc] = NULL;

        /*注1：ls命令自动添加参数--color，是为了实现真正shell执行ls命令的输出效果，
        --color是将目录与可执行文件采用不同的颜色显示
        注2：ps命令命令自动添加参数-t， 是为了实现真正shell执行ps命令的输出效果*/

        if(strcmp(temp[0], "ls") == 0){//判断是不是ls命令
            temp[targc]="--color";
            targc++;
            temp[targc] = NULL;
        }

        if(strcmp(temp[0], "ps") == 0){
            temp[targc]="-t";
            targc++;
            temp[targc] = NULL;
        }

        int exec_ret = execvp(temp[0], temp);
        
        if (exec_ret < 0){
            switch (errno)
            {
            case ENOENT: /* 要执行的命令文件不存在*/
                printf("%s: Command not found! \n", str);
                exit(0);
            case EACCES: /* 要执行的命令文件无访问权限*/
                printf("%s: permition Denied! \n", str);
                exit(0);
            }
        }    
    }
}

void solve(char* arvs){
    if (strstr(arvs, "|") == NULL){ //判断命令有没有管道,函数搜索一个字符串在另一个字符串中的第一次出现。
        solution(arvs);//无管道
        return;
    }

    // 有管道
    char* now;
    char* next = NULL;
    now = strtok_r(arvs, "|", &next); // 根据'|'符号，切割字符串,next指向剩余的子串。
    if (*next == 0){
        printf("The end need a filename after |.\n");
        exit(1);
    }

    now[strlen(now) - 1] = '\0';// 去除最后的空格
    
    for (int i = 0; i < strlen(next) - 1; i++)// 去除开头的空格
        next[i] = next[i + 1];
    next[strlen(next) - 1] = '\0';

    if (pipe(fildes) < 0){//创建管道
        printf("create pipe failed.\n");
        exit(1);
    }

    childpid = fork();
    if (childpid < 0){//创建子进程
        printf("create fork failed.\n");
        exit(1);
    }
    else if (childpid == 0) { // 将输出重定向为管道
        close(fildes[0]);
        dup2(fildes[1], 1); // 将文件标识符1和管道关联
        solution(now);
        exit(0);
    }
    else{

        waitpid(childpid, &status, 0);// 等待之前的命令完成
        close(fildes[1]); // 将输入重定向为管道
        dup2(fildes[0], 0);

        solve(next);// 递归调用该函数
        close(fildes[0]);
    }
}

int main(){
    parentpid = getpid();//获取进程的pid
    signal(SIGINT, (sighandler_t)sigcat); // 定义中断函数
    //char arvs[MAX_LINE];
    
    while(1){
        char *arvs;//用来存输入的命令
        //父进程自定义信号SIGINT的处理程序，当按下ctrl-c后，不终止父进程(Shell)，只终止其子程序。设置其它信号，当父进程(shell)收到该信号后，结束执行，退出
       
        background = 0;
        //printf("COMMAND->%d:", getpid()); // 命令提示符
        
        //fgets(arvs, MAX_LINE, stdin);//读取命令
        //fflush(stdin);//清空输入缓存
        char *dollar = "$ ";              //对于一般用户，用$提示
        char prompt[200] = UserNameColor; //绿色高亮度显示用户名
        char *UserName = getUserName();
        if (strcmp(UserName, "root") == 0)
           dollar = "# "; //对于root用户，用#提示
        strcat(prompt, UserName);
        strcat(prompt, ":");
        strcat(prompt, PathColor); //蓝色高亮度显示工作目录
        char *homeDirecrory = getUserHomeDirectory();
        // 用户主目录用显示为～
        char *path = SubStrReplace(getcwd(NULL, 0), homeDirecrory, "~");
        strcat(prompt, path);
        strcat(prompt, ColorEnd); //后续字符不采用色彩输出
        strcat(prompt, dollar);

        arvs = readline(prompt);

        add_history(arvs);

        if (strncmp(arvs, "cd", 2) == 0){
            Exec_Cd(arvs);
            continue;
        }

        char* AmperSand = strchr(arvs, '&');//一个进程结束了，但是它的父进程没有等待，那么它将会变成一个僵尸进程
        char Command[MAX_LINE + 1];

        for(int i = 0;i < MAX_LINE + 1; i++)
            Command[i]='\0';

        if (AmperSand != NULL && *(AmperSand + 1) != '>' && (strchr(arvs, '|') == NULL)){   // 结尾有&则后台执行
            /*要求后台运行*/
            background = 1;
            int n = strlen(arvs) - strlen(AmperSand);
            strncpy(Command, arvs, n);
            Command[n] = '\0';
            strcpy(Command, LRTrim(Command));  // 将command中的&去掉同时去掉空格
        }
        else{ /*不要求后台运行*/
            background = 0;
            strcpy(Command, LRTrim(arvs));
        } // if (AmperSand!=NULL)
        
        int stdinput = dup(0);// 保存默认输入输出
        int stdoutput = dup(1);

        solve(Command);

        if (background == 0)//如果不要求后台运行输入的命令，父进程等待子进程结束，否则，父进程不需要等待，继续执行，读入处理用户输入
            waitpid(childpid, &status, 0);

        dup2(stdinput, 0);// 恢复默认的输入输出
        dup2(stdoutput, 1);
    }
    return 0;
}
/*
    因为我们调用了./Test，所以我们在标准输入时如果不是命令会报错
    ./Test << 123   从标准输入读取直到读到123为止 
    ./Test < file.txt    将inFile.txt输入，输出到out.txt
    ./Test > out.txt < file.txt    将inFile.txt输入，输出到out.txt
    ./Test >> out.txt << 123    输出到out.txt追加
*/