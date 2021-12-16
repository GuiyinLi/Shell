/*
 * @Author: Ken Kaneki
 * @Date: 2021-12-13 09:06:58
 * @LastEditTime: 2021-12-15 14:55:20
 * @Description: README
 */
#ifndef SHELL_H_INCLUDED
#define SHELL_H_INCLUDED

#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <errno.h>

#define CLOSE "\001\033[0m\002"                        // 关闭所有属性
#define BLOD "\001\033[1m\002"                         // 强调、加粗、高亮
#define BEGINSHELL(x, y) "\001\033[" #x ";" #y "m\002" // x: 背景，y: 前景

#define MAX_CMD 10      // 最大命令数量
#define BUFFSIZE 255    // 输入最大命令字符数
#define MAX_CMD_LEN 100 // 每条命令的最大长度

typedef struct
{
    char *cmdline;       // 接受键盘输入的参数数组
    char *argv[MAX_CMD]; // 参数指针数组
    char argc;           // 命令行的有效参数个数
} Shell;

char *setPrompt();
int add_argv(char *str, Shell *shell); //将参数添加到参数数组
int do_list_cmd(char *argv[]);
int do_pipe_cmd(char argc, char *argv[]);
int do_simple_cmd(char argc, char *argv[], int prepipe[2], int postpipe[2]);
void reset_args(Shell *shell);

int callCd(char argc, char *argv[]);      // 执行cd指令
int ShowHistory(char argc, char *argv[]); //执行history指令

// 重定向指令
int commandWithOutputRedi(char argc, char *argv[], int division); // 执行输出重定向
int commandWithInputRedi(char argc, char *argv[], int division);  // 执行输入重定向命令
int commandWithReOutputRedi(char argc, char *argv[], int division);
int commandWithPipe(char buf[BUFFSIZE]);                        // 执行管道命令
int commandInBackground(char argc, char *argv[], int division); //执行后台运行指令

#endif
