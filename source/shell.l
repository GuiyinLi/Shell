%x QUOTE
%option noyywrap
%option nounput

%{
// 包含的C标准库的头文件
#include "shell.h"

// 外部变量声明，yytext为字符指针<QUOTE>"/n"     {BEGIN 0; do_list_cmd(); reset_args();}
// 这两个变量在lex提供的库中被定义，因此我们如果想要使用，必须添加extern声明"/n"            {do_list_cmd(); reset_args();}
extern char *yytext;
extern FILE *yyin;
Shell shell;
FILE *fp = NULL;

%}

%%
"\""                     {BEGIN QUOTE;}
<QUOTE>[^\n"]+           {add_argv(yytext,&shell);}
<QUOTE>"\""              {BEGIN 0;}
";"                      {add_argv(yytext,&shell);}
">"                      {add_argv(yytext,&shell);}
"<"                      {add_argv(yytext,&shell);}
"|"                      {add_argv(yytext,&shell);}
"&"                      {add_argv(yytext,&shell);}
">>"                     {add_argv(yytext,&shell);}
[^[:space:]\t\n|&<>>;"]+  {add_argv(yytext,&shell);}
.                       ;
%%
int main()
{
    while (1)
    {
        shell.cmdline = readline(setPrompt());

        add_history(shell.cmdline); // 加入历史列表

        if (!strlen(shell.cmdline)) // 没有命令则继续等待用户输入
            continue;

        fp = fopen("shell.txt", "w");
        fputs(shell.cmdline, fp);
        fclose(fp);

        yyin = fopen("shell.txt", "r");
        yylex();
        fclose(yyin);

        do_list_cmd(shell.argv);

        reset_args(&shell);
        free(shell.cmdline);
    }
    return 0;
}
