#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <pwd.h>
#include <wait.h>
#include <sys/types.h>
#include <readline/readline.h>
#include <readline/history.h>
///部分头文件的引入，此shell参考csdnmyShell进行实现，主要修改为
///history部分的加入，以及更完善的帮助信息更友好的提示信息等

//基础部分定义实现
char lastdir[100];
//定义目录结构
//定义一个过去历史过程中间的所有的命令集合，可以使用大概800左右的方法
char command[BUFSIZ]; 
//内存缓存的长度部分
char argv[100][100];
//转化为子命令的参数
char **argvtmp1;
//作为子命令临时间部分，如果遇到了类似管道符号的时候进行处理
char **argvtmp2;
char argv_redirect[100];
///定义是否进行重定向，如果进行了重定向的话，保留重定向的内容
int  argc;
///参数规模大小，split之后的参数数量
int BUILTIN_COMMAND = 0;
//内部实现exit，help，history等命令
int PIPE_COMMAND = 0;
//是否是有管道的命令
int REDIRECT_COMMAND = 0;
//是否有重定向的定义
void set_prompt(char *prompt); //部分定义的方法
//设置对应的使用shell的人的部分参数，类似linux环境下面的用户信息提醒
int analysis_command();
//分析命令行的结构，判断是否属于有管道符号等
void builtin_command();
//是否是内建命令，如何是内建命令的话，使用此函数进行处理
void do_command();
//不是内建命令，使用对应的execvp函数进行处理，其参数为【应用程序名称，参数列表】两个部分
void help();
//内建对应的help命令，感觉不合理
void initial();
//每一次命令再次输入的时候重置整个共有变量
void init_lastdir();
///初始化现在的路径参数
void history_setup();
//使用readline动态库中间的history函数进行历史列表的获取
void history_finish();
//保存对应的历史列表
void display_history_list();
//类似现在的linux系统下面的history命令的历史输出

