#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <dirent.h>

#define MAXLINE 100
#define MAXARGS 100
#define M 100

char his[M][M];
int his_cnt = 0; //历史命令计数
char *path = NULL;

void doCommand(char *cmdline);
int parseline(const char *cmdline, char **argv);
int builtin_cmd(char **argv);
void pipe_line(char *process1[], char *process2[]);
void mytop();

int main(int argc, char **argv)
{
    char c;
    char cmdline[MAXLINE];

    while (1)
    {
        path = getcwd(NULL, 0);
        printf("10215501437Tangxiaohui_shell>%s# ", path);
        fflush(stdout);
        if (fgets(cmdline, MAXLINE, stdin) == NULL)
        {
            continue;
        }
        for (int i = 0; i < M; i++)
        {
            his[his_cnt][i] = cmdline[i];
        }
        his_cnt = his_cnt + 1;
        doCommand(cmdline);
        fflush(stdout);
    }
    exit(0);
}

void doCommand(char *cmdline)
//内置命令、program命令、后台运行
{
    char *argv[MAXARGS];
    char buf[MAXLINE];
    int bg;
    pid_t pid;
    char *file;
    int fd;
    int status;
    int case_command = 0;
    strcpy(buf, cmdline);

    if ((bg = parseline(buf, argv)) == 1)
    { //后台
        case_command = 4;
    }
    if (argv[0] == NULL)
    {
        return;
    }

    if (builtin_cmd(argv))
        return; //内置命令返回

    int i = 0;
    for (i = 0; argv[i] != NULL; i++)
    {
        if (strcmp(argv[i], ">") == 0)
        {
            if (strcmp(argv[i + 1], ">") == 0)
            {
                case_command = 5;
                break;
            }
            case_command = 1;
            file = argv[i + 1];
            argv[i] = NULL;
            break;
        }
    }

    for (i = 0; argv[i] != NULL; i++)
    {
        if (strcmp(argv[i], "<") == 0)
        {
            case_command = 2;
            file = argv[i + 1];
            printf("filename=%s\n", file);
            argv[i] = NULL;
            break;
        }
    }
    char *leftargv[MAXARGS];
    for (i = 0; argv[i] != NULL; i++)
    {
        if (strcmp(argv[i], "|") == 0)
        {
            case_command = 3;
            argv[i] = NULL;
            int j;
            for (j = i + 1; argv[j] != NULL; j++)
            {
                leftargv[j - i - 1] = argv[j];
            }
            leftargv[j - i - 1] = NULL;
            break;
        }
    }

    switch (case_command)
    {
    case 0:
        if ((pid = fork()) == 0)
        {
            execvp(argv[0], argv);
            exit(0);
        }
        if (waitpid(pid, &status, 0) == -1)
        {
            printf("error\n");
        }
        break;
    case 1:
        /*包含重定向输出*/
        if ((pid = fork()) == 0)
        {
            fd = open(file, O_RDWR | O_CREAT | O_TRUNC, 7777);
            if (fd == -1)
            {
                printf("open %s error!\n", file);
            }
            dup2(fd, 1);
            close(fd);
            execvp(argv[0], argv);
            exit(0);
        }
        if (waitpid(pid, &status, 0) == -1)
        {
            printf("error\n");
        }
        break;
    case 2:
        /*包含重定向输入*/
        if ((pid = fork()) == 0)
        {
            fd = open(file, O_RDONLY);
            dup2(fd, 0);
            close(fd);
            execvp(argv[0], argv);
            exit(0);
        }
        if (waitpid(pid, &status, 0) == -1)
        {
            printf("error\n");
        }
        break;
    case 3:
        /*命令包含管道*/
        if ((pid = fork()) == 0)
        {
            pipe_line(argv, leftargv);
        }
        else
        {
            if (waitpid(pid, &status, 0) == -1)
            {
                printf("error\n");
            }
        }
        break;
    case 4: //后台运行
        signal(SIGCHLD, SIG_IGN);
        if ((pid = fork()) == 0)
        {
            signal(SIGCHLD, SIG_IGN);
            close(0);
            open("/dev/null", O_RDONLY);
            close(1);
            open("/dev/null", O_WRONLY);
            execvp(argv[0], argv);
            exit(0);
        }
        //不等待结束
        break;
    case 5: //追加写
        if ((pid = fork()) == 0)
        {
            fd = open(file, O_RDWR | O_CREAT | O_APPEND, 7777);
            if (fd == -1)
            {
                printf("open %s error!\n", file);
            }
            dup2(fd, 1);
            close(fd);
            execvp(argv[0], argv);
            exit(0);
        }
        if (waitpid(pid, &status, 0) == -1)
        {
            printf("error\n");
        }
        break;

    default:
        break;
    }
    return;
}

int parseline(const char *cmdline, char **argv)
{
    static char array[MAXLINE];
    char *buf = array;
    int argc = 0;
    int bg;

    strcpy(buf, cmdline);
    buf[strlen(buf) - 1] = ' ';
    while (*buf && (*buf == ' '))
        buf++;

    char *s = strtok(buf, " ");
    if (s == NULL)
    {
        exit(0);
    }
    argv[argc] = s;
    argc++;
    while ((s = strtok(NULL, " ")) != NULL)
    {
        argv[argc] = s;
        argc++;
    }
    argv[argc] = NULL;

    if (argc == 0)
        return 1;

    if ((bg = (*argv[(argc)-1] == '&')) != 0)
    {
        argv[--(argc)] = NULL;
    }
    return bg;
}


void mytop() {
    FILE *fp = NULL;            
    char buff[255];

    fp = fopen("/proc/meminfo", "r");   // 以只读方式打开meminfo文件
    fgets(buff, 255, (FILE*)fp);        // 读取meminfo文件内容进buff
    fclose(fp);

    // 获取 pagesize
    int i = 0, pagesize = 0;
    while (buff[i] != ' ') {
        pagesize = 10 * pagesize + buff[i] - 48;
        i++;
    }

    // 获取 页总数 total
    i++;
    int total = 0;
    while (buff[i] != ' ') {
        total = 10 * total + buff[i] - 48;
        i++;
    }

    // 获取空闲页数 free
    i++;
    int free = 0;
    while (buff[i] != ' ') {
        free = 10 * free + buff[i] - 48;
        i++;
    }

    // 获取最大页数量largest
    i++;
    int largest = 0;
    while (buff[i] != ' ') {
        largest = 10 * largest + buff[i] - 48;
        i++;
    }

    // 获取缓存页数量 cached
    i++;
    int cached = 0;
    while (buff[i] >= '0' && buff[i] <= '9') {
        cached = 10 * cached + buff[i] - 48;
        i++;
    }

    // 总体内存大小 = (pagesize * total) / 1024 单位 KB
    int totalMemory  = pagesize / 1024 * total;
    // 空闲内存大小 = pagesize * free) / 1024 单位 KB
    int freeMemory   = pagesize / 1024 * free;
    // 缓存大小    = (pagesize * cached) / 1024 单位 KB
    int cachedMemory = pagesize / 1024 * cached;

    printf("totalMemory  is %d KB\n", totalMemory);
    printf("freeMemory   is %d KB\n", freeMemory);
    printf("cachedMemory is %d KB\n", cachedMemory);

    /* 2. 获取内容2
        进程和任务数量
     */
    fp = fopen("/proc/kinfo", "r");     // 以只读方式打开meminfo文件
    memset(buff, 0x00, 255);            // 格式化buff字符串
    fgets(buff, 255, (FILE*)fp);        // 读取meminfo文件内容进buff
    fclose(fp);

    // 获取进程数量
    int processNumber = 0;
    i = 0;
    while (buff[i] != ' ') {
        processNumber = 10 * processNumber + buff[i] - 48;
        i++;
    }
    printf("processNumber = %d\n", processNumber);

    // 获取任务数量
    i++;
    int tasksNumber = 0;
    while (buff[i] >= '0' && buff[i] <= '9') {
        tasksNumber = 10 * tasksNumber + buff[i] - 48;
        i++;
    }
    printf("tasksNumber = %d\n", tasksNumber);
    return;
}

int builtin_cmd(char **argv)
{
    //内置命令
    if (!strcmp(argv[0], "exit"))
    {
        exit(0);
    }
    if (!strcmp(argv[0], "mytop"))
    {
        mytop();
        return 1;
    }
    if (!strcmp(argv[0], "cd"))
    {
        if (!argv[1])
        { // cd后面啥也没有
            argv[1] = ".";
        }
        int ret = chdir(argv[1]); //改变目录
        if (ret < 0)
        {
            printf("No such directory!\n");
        }
        else
        {
            path = getcwd(NULL, 0); //当前目录
        }
        return 1;
    }

    if (!strcmp(argv[0], "history"))
    {
        if (!argv[1])
        { //只输入history
            for (int j = 1; j <= his_cnt; j++)
            {
                printf("%d ", j);
                puts(his[j - 1]);
            }
        }
        else
        {
            int t = atoi(argv[1]);
            if (his_cnt - t < 0)
            {
                printf("please confirm the number below %d\n", his_cnt);
            }
            else
            {
                for (int j = his_cnt - t; j < his_cnt; j++)
                {
                    printf("%d ", j + 1);
                    puts(his[j]);
                }
            }
        }
        return 1;
    }

    return 0;
}

void pipe_line(char *process1[], char *process2[])
{
    int fd[2];
    pipe(&fd[0]);

    int status;
    pid_t pid = fork();
    if (pid == 0)
    {
        close(fd[0]);
        close(1);
        dup(fd[1]);
        close(fd[1]);
        execvp(process1[0], process1);
    }
    else
    { //父进程中
        close(fd[1]);
        close(0);
        dup(fd[0]);
        close(fd[0]);
        waitpid(pid, &status, 0);
        execvp(process2[0], process2);
    }
}