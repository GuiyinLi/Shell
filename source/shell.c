/*
 * @Author: Ken Kaneki
 * @Date: 2021-12-15 12:41:09
 * @LastEditTime: 2021-12-15 15:22:38
 * @Description: README
 */
#include "shell.h"

/**
 * @description: 设置shell提示符(当前路径)及格式
 * @param {void}
 * @return {char *}
 */
char *setPrompt(void)
{
    static char prompt[2 * BUFFSIZE];
    static char curpath[BUFFSIZE];
    memset(prompt, 0, 2 * BUFFSIZE);
    strcat(prompt, BEGINSHELL(49, 35));
    strcat(prompt, BLOD);
    getcwd(curpath, BUFFSIZE);
    strcat(prompt, curpath);
    strcat(prompt, " $ ");
    strcat(prompt, CLOSE);
    strcat(prompt, "\0");
    return prompt;
}

int add_argv(char *str, Shell *shell)
{
    shell->argv[shell->argc] = malloc(strlen(str) * sizeof(char));
    strcpy(shell->argv[shell->argc++], str);
    return 0;
}

int do_list_cmd(char *argv[])
{
    int i = 0;
    int j = 0;
    char *p;
    while (argv[i])
    {
        if (strcmp(argv[i], ";") == 0)
        {
            p = argv[i];
            argv[i] = 0;
            do_pipe_cmd(i - j, argv + j); //参数个数 首地址
            argv[i] = p;
            j = ++i;
        }
        else
            i++;
    }
    do_pipe_cmd(i - j, argv + j);
    return 0;
}

int do_pipe_cmd(char argc, char *argv[])
{
    int i = 0, j = 0, prepipe = 0;
    int prefd[2], postfd[2];
    char *p;
    while (argv[i])
    {
        if (strcmp(argv[i], "|") == 0)
        { // pipe
            p = argv[i];
            argv[i] = 0;
            pipe(postfd); // create the post pipe
            if (prepipe)
                do_simple_cmd(i - j, argv + j, prefd, postfd);
            else
                do_simple_cmd(i - j, argv + j, 0, postfd);
            argv[i] = p;
            prepipe = 1;
            prefd[0] = postfd[0];
            prefd[1] = postfd[1];
            j = ++i;
        }
        else
            i++;
    }
    if (prepipe)
        do_simple_cmd(i - j, argv + j, prefd, 0);
    else
        do_simple_cmd(i - j, argv + j, 0, 0);
    return 0;
}

int do_simple_cmd(char argc, char *argv[], int prepipe[2], int postpipe[2])
{
    pid_t pid;
    int j;

    for (j = 0; j < argc; j++)
    {
        if (strcmp(argv[j], ">") == 0)
        {
            return commandWithOutputRedi(argc, argv, j);
        }
    }

    // 识别输入重定向
    for (j = 0; j < argc; j++)
    {
        if (strcmp(argv[j], "<") == 0)
        {
            return commandWithInputRedi(argc, argv, j);
        }
    }

    // 识别追加写重定向
    for (j = 0; j < argc; j++)
    {
        if (strcmp(argv[j], ">>") == 0)
        {
            return commandWithReOutputRedi(argc, argv, j);
        }
    }

    // 识别后台运行
    for (j = 0; j < argc; j++)
    {
        if (strcmp(argv[j], "&") == 0)
        {
            return commandInBackground(argc, argv, j);
        }
    }

    /* 识别shell内置命令 */
    if (strcmp(argv[0], "cd") == 0)
    {
        return callCd(argc, argv);
    }

    else if (strcmp(argv[0], "history") == 0)
    {
        return ShowHistory(argc, argv);
    }

    else if (strcmp(argv[0], "exit") == 0)
    {
        exit(0);
    }

    else
    {
        switch (pid = fork())
        {
        // fork子进程失败
        case -1:
            printf("creat subprocess failed\n");
            return 1;
        // 处理子进程
        case 0:
        { /* 函数说明：execvp()会从PATH 环境变量所指的目录中查找符合参数file 的文件名, 找到后便执行该文件,
           * 然后将第二个参数argv 传给该欲执行的文件。
           * 返回值：如果执行成功则函数不会返回, 执行失败则直接返回-1, 失败原因存于errno 中.
           * */
            execvp(argv[0], argv);
            // 代码健壮性: 如果子进程未被成功执行, 则报错
            printf("%s", errno);
            // exit函数终止当前进程, 括号内参数为1时, 会像操作系统报告该进程因异常而终止
            exit(1);
        }
        default:
        {
            int status;
            waitpid(pid, &status, 0);      // 等待子进程返回
            int err = WEXITSTATUS(status); // 读取子进程的返回码
            if (err)
            {
                printf("Error: %s\n", strerror(err));
                return 1;
            }
        }
        }
    }

    return 0;
}

int callCd(char argc, char *argv[])
{
    if (argc == 1)
    {
        printf("%s: too few arguments\n", argv[0]);
        return 1;
    }
    if (argc > 2)
    {
        printf("%s: too many arguments\n", argv[0]);
        return 1;
    }
    else
    {
        if (chdir(argv[1]))
        {
            printf("No such file or directory!\n");
            return 1;
        }
    }
    return 0;
}

int ShowHistory(char argc, char *argv[])
{
    if (argc > 2)
    {
        printf("%s: too many arguments\n", argv[0]);
        return 1;
    }
    int i = 0;
    int n = 0;
    int j = 0;
    HIST_ENTRY **his;
    his = history_list();

    if (argc == 2)
    {
        for (i = 0; i < strlen(argv[1]); i++)
        {
            if (!(argv[1][i] >= '0' && argv[1][i] <= '9'))
            {
                printf("%s: numeric argument required\n", argv[1]);
                return 1;
            }
        }

        i = 0;
        n = atoi(argv[1]);
        while (his[i] != NULL)
        {
            i++;
        }
    }

    else
        n = 0;

    j = i - n - 1;
    if (j < 0)
        j = 0;
    while (his[j] != NULL)
    {
        printf("%d %s\n", j + 1, his[j]->line);
        j++;
    }
}

int commandWithOutputRedi(char argc, char *argv[], int division)
{

    int pos[MAX_CMD] = {0};
    char *q;
    int i, j = 0;
    for (i = division; i < argc; i++)
    {
        if (strcmp(argv[i], ">") == 0)
        {
            if (i + 1 < argc)
            {
                pos[j] = i + 1;
                j++;
            }
            else
            {
                printf("syntax error near unexpected token newline!\n");
                return 1;
            }
        }
    }
    pid_t pid;
    int fd;
    for (i = 0; i < j; i++)
    {
        switch (pid = fork())
        {
        case -1:
        {
            printf("creat subprocess failed\n");
            return 1;
        }
        // 处理子进程:
        case 0:
        {

            fd = open(argv[pos[i]], O_WRONLY | O_CREAT | O_TRUNC);
            char *cmd;
            sprintf(cmd, "chmod 777 %s", argv[pos[i]]);
            system(cmd);
            if (fd < 0)
            {
                exit(1);
            }

            dup2(fd, STDOUT_FILENO);
            q = argv[division];
            argv[division] = NULL;
            execvp(argv[0], argv);
            if (fd != STDOUT_FILENO)
            {
                close(fd);
            }
            argv[division] = q;
            printf("error:%s", errno); // 代码健壮性: 如果子进程未被成功执行, 则报错
            exit(1);                   // exit函数终止当前进程, 括号内参数为1时, 会像操作系统报告该进程因异常而终止
        }
        default:
        {
            int status;
            waitpid(pid, &status, 0);      // 等待子进程返回
            int err = WEXITSTATUS(status); // 读取子进程的返回码
            if (err)
            {
                printf("Error: %s\n", strerror(err));
                return 1;
            }
        }
        }
    }

    return 0;
}

int commandWithInputRedi(char argc, char *argv[], int division)
{
    int pos[MAX_CMD] = {0};
    char *q;
    int i, j = 0;
    for (i = division; i < argc; i++)
    {
        if (strcmp(argv[i], "<") == 0)
        {
            if (i + 1 < argc)
            {
                pos[j] = i + 1;
                j++;
            }
            else
            {
                printf("syntax error near unexpected token newline!\n");
                return 1;
            }
        }
    }
    pid_t pid;
    int fd;
    for (i = 0; i < j; i++)
    {
        switch (pid = fork())
        {
        case -1:
        {
            printf("creat subprocess failed\n");
            return 1;
        }
        // 处理子进程:
        case 0:
        {
            fd = open(argv[pos[i]], O_RDONLY);
            char *cmd;
            sprintf(cmd, "chmod 777 %s", argv[pos[i]]);
            system(cmd);
            if (fd < 0)
            {
                exit(1);
            }

            dup2(fd, STDIN_FILENO);
            q = argv[division];
            argv[division] = NULL;
            execvp(argv[0], argv);
            if (fd != STDIN_FILENO)
            {
                close(fd);
            }
            argv[division] = q;
            printf("error:%s", errno); // 代码健壮性: 如果子进程未被成功执行, 则报错
            exit(1);                   // exit函数终止当前进程, 括号内参数为1时, 会像操作系统报告该进程因异常而终止
        }
        default:
        {
            int status;
            waitpid(pid, &status, 0);      // 等待子进程返回
            int err = WEXITSTATUS(status); // 读取子进程的返回码
            if (err)
            {
                printf("Error: %s\n", strerror(err));
                return 1;
            }
        }
        }
    }

    return 0;
}

int commandWithReOutputRedi(char argc, char *argv[], int division)
{

    int pos[MAX_CMD] = {0};
    char *q;
    int i, j = 0;
    for (i = division; i < argc; i++)
    {
        if (strcmp(argv[i], ">>") == 0)
        {
            if (i + 1 < argc)
            {
                pos[j] = i + 1;
                j++;
            }
            else
            {
                printf("syntax error near unexpected token newline!\n");
                return 1;
            }
        }
    }
    pid_t pid;
    int fd;
    for (i = 0; i < j; i++)
    {
        switch (pid = fork())
        {
        case -1:
        {
            printf("creat subprocess failed\n");
            return 1;
        }
        // 处理子进程:
        case 0:
        {
            fd = open(argv[pos[i]], O_WRONLY | O_APPEND | O_CREAT);
            char *cmd;
            sprintf(cmd, "chmod 777 %s", argv[pos[i]]);
            system(cmd);
            if (fd < 0)
            {
                exit(1);
            }

            dup2(fd, STDOUT_FILENO);
            q = argv[division];
            argv[division] = NULL;
            execvp(argv[0], argv);
            if (fd != STDOUT_FILENO)
            {
                close(fd);
            }
            argv[division] = q;
            printf("error:%s", errno); // 代码健壮性: 如果子进程未被成功执行, 则报错
            exit(1);                   // exit函数终止当前进程, 括号内参数为1时, 会像操作系统报告该进程因异常而终止
        }
        default:
        {
            int status;
            waitpid(pid, &status, 0);      // 等待子进程返回
            int err = WEXITSTATUS(status); // 读取子进程的返回码
            if (err)
            {
                printf("Error: %s\n", strerror(err));
                return 1;
            }
        }
        }
    }

    return 0;
}

int commandInBackground(char argc, char *argv[], int division)
{
    char *q;
    pid_t pid;
    pid = fork();
    if (pid < 0)
    {
        perror("fork()");
        exit(1);
    }
    else if (pid == 0)
    {

        // 将stdin、stdout、stderr重定向到/dev/null
        freopen("/dev/null", "w", stdout);
        freopen("/dev/null", "r", stdin);

        signal(SIGCHLD, SIG_IGN); //  子进程调用signal(SIGCHLD,SIG_IGN)，使得Linux接管此进程

        q = argv[division];
        argv[division] = NULL;
        execvp(argv[0], argv);
        argv[division] = q;
        printf("%s", errno);

        // exit函数终止当前进程, 括号内参数为1时, 会像操作系统报告该进程因异常而终止
        exit(1);
    }
    else
    {
        // 父进程不等待子进程结束就返回
        // exit(0);
        return 0;
    }
}

void reset_args(Shell *shell)
{
    while (--shell->argc)
    {
        shell->argv[shell->argc] = NULL;
    }
}
