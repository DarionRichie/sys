#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<dirent.h>
#include<pwd.h>
#include<wait.h>
#include<sys/types.h>
#include <readline/readline.h>
#include <readline/history.h>

//网上代码进行借鉴的，但是魔改了部分的东西

//define the printf color
#define L_GREEN "\e[1;32m"  //定义答应过程中间的颜色，这是绿色
#define L_BLUE  "\e[1;34m"
#define L_RED   "\e[1;31m"
#define WHITE   "\e[0m"

#define TRUE 1
#define FALSE 0
#define DEBUG //标识对应的gdb debug调试时候才会进行展示

char lastdir[100]; //上一次的内容？
//定义一个过去历史过程中间的所有的命令集合，可以使用大概800左右的方法
char command[BUFSIZ]; //内存缓存的长度部分
char argv[100][100];//转化为子命令的参数
char **argvtmp1;//作为子命令临时？
char **argvtmp2;
char argv_redirect[100];
int  argc;
int BUILTIN_COMMAND = 0;
int PIPE_COMMAND = 0;
int REDIRECT_COMMAND = 0;
//set the prompt
void set_prompt(char *prompt); //部分定义的方法
//analysis the command that user input
int analysis_command();
void builtin_command();
void do_command();
//print help information
void help();
void initial();
void init_lastdir();
void history_setup();
void history_finish();
void display_history_list();

int main(){
	char prompt[BUFSIZ]; //定义一个缓存区大小的字符数组
	char *line; //
	
	init_lastdir(); //初始化对应的目录？
	history_setup();	//开始对应的历史纪录吧？
	while(1) {
		set_prompt(prompt);
		if(!(line = readline(prompt))) //读取一个命令开始,只是有个提示对吧
			break;
		// printf("-----------%s,%s-----------------\n",line,prompt);
		if(*line)
			add_history(line);//加入到历史数据中间
		// display_history_list();
		strcpy(command, line);//将对应的命令复制到command数组里面
		//strcat(command, "\n");
		if(!(analysis_command())){ //分析对应的命令输入参数是否可行
			//todo deal with the buff
			if(BUILTIN_COMMAND){
				builtin_command();	//判断是否是内部命令
			}//if
			else{
				do_command(); //如果不是对应的命令，进行自定义的命令参数
			}//else		
		}//if analysis_command
		initial();//initial 
	}//while
	history_finish();
	
	return 0;
}

//set the prompt
void set_prompt(char *prompt){
	char hostname[100];
	char cwd[100];
	char super = '#';
	//to cut the cwd by "/"	
	char delims[] = "/";	
	struct passwd* pwp;
	
	if(gethostname(hostname,sizeof(hostname)) == -1){
		//get hostname failed
		//获取对应的主机名字，可以进行打印尝试
		// printf("%s-------------------这是我自己的主机的名字","hostname");
		strcpy(hostname,"unknown");
	}//if
	//getuid() get user id ,then getpwuid get the user information by user id 
	pwp = getpwuid(getuid());	
	//这里是打印对应的用户的信息，使用内置的结构体进行
	// printf("username:%s\n",pwp->pw_name);
	// printf("user password:%s\n",pwp->pw_passwd);
	// printf("user ID:%d\n",pwp->pw_uid);
	// printf("group ID:%d\n",pwp->pw_gid);
	// printf("real name:%s\n",pwp->pw_gecos);
	// printf("home directory:%s\n",pwp->pw_dir);
	// printf("shell program:%s\n",pwp->pw_shell);
	/////////////////////////////
	if(!(getcwd(cwd,sizeof(cwd)))){
		//get cwd failed
		strcpy(cwd,"unknown");	//获取当前的工作的目录，进行分析
	}//if
	char cwdcopy[100];
	strcpy(cwdcopy,cwd);
	//可以进行打印查看对应的消息
	// printf("%s----这是我获取的对应的路径位置",cwdcopy);
	char *first = strtok(cwdcopy,delims);
	char *second = strtok(NULL,delims);
	//获取对应的下的一个位置的值

	// printf("%s----home这是我获取的对应的路径位置",first);
	// printf("%s----%s这是我获取的对应的路径位置",second,pwp->pw_name);

	//if at home 

	if(!(strcmp(first,"home")) && !(strcmp(second,pwp->pw_name))){
		// printf("1111111111111111111111");
		//如果是home 目录，并且对应的用户权限是一致的话
		int offset = strlen(first) + strlen(second)+2;//做偏移
		char newcwd[100];
		char *p = cwd;
		char *q = newcwd;

		p += offset;
		while(*(q++) = *(p++));
		char tmp[100];
		strcpy(tmp,"~");
		strcat(tmp,newcwd);
		strcpy(cwd,tmp);			
	}	
	
	if(getuid() == 0)//if super
		super = '#';//这里表面是对应的超级用户呀
	else
		super = '$';
	// printf("\n路径%s路口/n/n\n\n\n",prompt);
	//只是把对应的文件名啥的写进来而已
	sprintf(prompt, "\n\001\e[1;32m\002%s@%s\001\e[0m\002:\001\e[1;31m\002%s\001\e[0m\002%c",pwp->pw_name,hostname,cwd,super);	
	
}

//analysis command that user input
//分析对应的用户的输入是怎么样的
int analysis_command(){    
	int i = 1;
	char *p;
	//to cut the cwd by " "	
	char delims[] = " ";
	argc = 1;
	//使用argv[0]其实感觉很离谱呀，进行打印一下
	// printf("-----这是我打印的参数1---%s%n",command);
	strcpy(argv[0],strtok(command,delims)); //c没有split使用strtok进行对应的切分呀
	while(p = strtok(NULL,delims)){
		strcpy(argv[i++],p);
		argc++;
		//把对应的参数全部都取出来呀
	}//while
	
	if(!(strcmp(argv[0],"exit"))||!(strcmp(argv[0],"help1"))|| !(strcmp(argv[0],"cd")) || !(strcmp(argv[0],"history"))){ //这里可以加入对应的历史列表的函数进行触发
		BUILTIN_COMMAND = 1;	
	}
	int j;
	//is a pipe command ?
	int pipe_location;
	for(j = 0;j < argc;j++){
		if(strcmp(argv[j],"|") == 0){
			PIPE_COMMAND = 1;
			pipe_location = j;				
			break;
			//查看是否是管道符
		}	
	}//for
	
	//is a redirect command ?
	int redirect_location;
	for(j = 0;j < argc;j++){
		if(strcmp(argv[j],">") == 0){
			REDIRECT_COMMAND = 1;
			redirect_location = j;		
			//是不是对应的重定向jian

			break;
		}
	}//for

	if(PIPE_COMMAND){
		//command 1
		argvtmp1 = malloc(sizeof(char *)*pipe_location + 1);
		int i;	
		for(i = 0;i < pipe_location + 1;i++){
			argvtmp1[i] = malloc(sizeof(char)*100);
			if(i <= pipe_location)
				strcpy(argvtmp1[i],argv[i]);	
		}//for
		argvtmp1[pipe_location] = NULL;
		
		//command 2
		argvtmp2 = malloc(sizeof(char *)*(argc - pipe_location));
		int j;	
		for(j = 0;j < argc - pipe_location;j++){
			argvtmp2[j] = malloc(sizeof(char)*100);
			if(j <= pipe_location)
				strcpy(argvtmp2[j],argv[pipe_location + 1 + j]);	
		}//for
		argvtmp2[argc - pipe_location - 1] = NULL;
		
	}//if pipe_command

	else if(REDIRECT_COMMAND){
		strcpy(argv_redirect,argv[redirect_location + 1]);
		argvtmp1 = malloc(sizeof(char *)*redirect_location + 1);
		int i;	
		for(i = 0;i < redirect_location + 1;i++){
			argvtmp1[i] = malloc(sizeof(char)*100);
			if(i < redirect_location)
				strcpy(argvtmp1[i],argv[i]);	
		}//for
		argvtmp1[redirect_location] = NULL;
	}//redirect command

	else{
		argvtmp1 = malloc(sizeof(char *)*argc+1);
		int i;	
		for(i = 0;i < argc + 1;i++){
			argvtmp1[i] = malloc(sizeof(char)*100);
			if(i < argc)
				strcpy(argvtmp1[i],argv[i]);	
		}//for
		argvtmp1[argc] = NULL;
	}
	

#ifdef DEBUG
	//test the analysis		
	if(BUILTIN_COMMAND){
		printf("\tthis is a builtin command: %s\n",argv[0]);
	}
	else if(PIPE_COMMAND){
		printf("\tthis is a pipe command:\n");
		printf("\t==command 1:\n");		
		int k;	
		for(k = 0;k < pipe_location + 1;k++){
			printf("\t%d: %s\n",k,argvtmp1[k]);	
		}//for
		printf("\t==command 2:\n");
			for(k = 0;k < argc - pipe_location;k++){
			printf("\t%d: %s\n",k,argvtmp2[k]);	
		}//for	
	}
	else if(REDIRECT_COMMAND){
		printf("\tthis is a redirect command:\n");
		printf("\t==command:\n");		
		int k;	
		for(k = 0;k < pipe_location + 1;k++){
			printf("\t%d: %s\n",k,argvtmp1[k]);	
		}//for
		printf("redirect target: %s\n",argv_redirect);
	}
	else{
		printf("\nthe command is:%s with %d parameter(s):\n",argv[0],argc);
		printf("0(command): %s\n",argv[0]);	
		int k;	
		for(k = 1;k < argc;k++){
			printf("%d: %s\n",k,argv[k]);	
		}//for
	}
			
#endif	

	return 0;
}

void builtin_command(){
	struct passwd* pwp;
	//exit when command is exit	
	if(strcmp(argv[0],"exit") == 0){
		exit(EXIT_SUCCESS);
	}
	else if(strcmp(argv[0],"help") == 0){
		help();
	}//else if
	else if(strcmp(argv[0],"history") == 0){
		display_history_list();
	}
	else if(strcmp(argv[0],"cd") == 0){
		char cd_path[100];
		if((strlen(argv[1])) == 0 ){
			pwp = getpwuid(getuid());
			sprintf(cd_path,"/home/%s",pwp->pw_name);
			strcpy(argv[1],cd_path);
			argc++;			
		}
		else if((strcmp(argv[1],"~") == 0) ){
			pwp = getpwuid(getuid());
			sprintf(cd_path,"/home/%s",pwp->pw_name);
			strcpy(argv[1],cd_path);			
		}
	
		//do cd
#ifdef DEBUG	
		printf("cdpath = %s \n",argv[1]);	
#endif		
		if((chdir(argv[1]))< 0){
			printf("cd failed in builtin_command()\n");
		}
	}//else if cd
}

void do_command(){
	//do_command
	
	if(PIPE_COMMAND){
		int fd[2],res;
		int status;
		
		res = pipe(fd);
	
		if(res == -1)
			printf("pipe failed in do_command()\n");
		pid_t pid1 = fork();
		if(pid1 == -1){
			printf("fork failed in do_command()\n");		
		}//if
		else if(pid1 == 0){
				dup2(fd[1],1);//dup the stdout
				close(fd[0]);//close the read edge
				if(execvp(argvtmp1[0],argvtmp1) < 0){
					#ifdef DEBUG
					printf("execvp failed in do_command() !\n");
					#endif
					printf("%s:command not found\n",argvtmp1[0]);		
				}//if			
		}//else if child pid1
		else{
			waitpid(pid1,&status,0);
			pid_t pid2 = fork();
			if(pid2 == -1){
				printf("fork failed in do_command()\n");		
			}//if
			else if(pid2 == 0){
				close(fd[1]);//close write edge
				dup2(fd[0],0);//dup the stdin
				if(execvp(argvtmp2[0],argvtmp2) < 0){
					#ifdef DEBUG
					printf("execvp failed in do_command() !\n");
					#endif
					printf("%s:command not found\n",argvtmp2[0]);		
				}//if
			}//else if pid2 == 0
			else{
				close(fd[0]);
				close(fd[1]);
				waitpid(pid2,&status,0);
			}//else 
		}//else parent process
	}//if pipe command

	else if(REDIRECT_COMMAND){
		pid_t pid = fork();	
		if(pid == -1){
			printf("fork failed in do_command()\n");		
		}//if
		else if(pid == 0){
			int redirect_flag = 0;
			FILE* fstream;
			fstream = fopen(argv_redirect,"w+");
			freopen(argv_redirect,"w",stdout);
			if(execvp(argvtmp1[0],argvtmp1) < 0){
				redirect_flag = 1;//execvp this redirect command failed		
			}//if
			fclose(stdout);
			fclose(fstream);
			if(redirect_flag){
				#ifdef DEBUG
				printf("execvp redirect command failed in do_command() !\n");
				#endif
				printf("%s:command not found\n",argvtmp1[0]);
			}//redirect flag	
				
		}//else if 
		else{
			int pidReturn = wait(NULL);	
		}//else  
	}//else if redirect command 
	else{
		pid_t pid = fork();	
		if(pid == -1){
			printf("fork failed in do_command()\n");		
		}//if
		else if(pid == 0){
			if(execvp(argvtmp1[0],argvtmp1) < 0){
				#ifdef DEBUG
				printf("execvp failed in do_command() !\n");
				#endif
				printf("%s:command not found\n",argvtmp1[0]);			
			}//if	
		}//else if 
		else{
			int pidReturn = wait(NULL);	
		}//else  
	}//else normal command

        free(argvtmp1);
	free(argvtmp2);
}

void help(){
		char message[50] = "Hi,welcome to dtwang_Shell!";
		printf(
"< %s >\n"
"\t\t\\\n"
"\t\t \\   \\_\\_    _/_/\n"
"\t\t  \\      \\__/\n"
"\t\t   \\     (oo)\\_______\n"
"\t\t    \\    (__)\\       )\\/\\\n"
"\t\t             ||----w |\n"
"\t\t             ||     ||\n",message);
}

void initial(){
	int i = 0;	
	for(i = 0;i < argc;i++){
		strcpy(argv[i],"\0");	
	}
	argc = 0;
	BUILTIN_COMMAND = 0;
	PIPE_COMMAND = 0;
	REDIRECT_COMMAND = 0;
}

void init_lastdir(){
	getcwd(lastdir, sizeof(lastdir)); //获取对应的大小的 getcwd()会将当前工作目录的绝对路径复制到参数buffer所指的内存空间中
	printf("+++++++++++++++++++++++++当前的文件路径位置%s%n++++++++++++++++++++++++++++",lastdir);
	printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
}

void history_setup(){
	//开始记录对应的历史纪录，开始离谱了呀
	using_history();
	stifle_history(50);//纪录最大的列表
	//这个库的目的是使用输入的历史纪录吗 离谱了
	read_history("/tmp/msh_history");	
}

void history_finish(){
	append_history(history_length, "/tmp/msh_history");
	history_truncate_file("/tmp/msh_history", history_max_entries);
}

void display_history_list(){
	//做对应的命令列表的输出
	HIST_ENTRY** h = history_list();
	if(h) {
		int i = 0;
		while(h[i]) {
			printf("%d: %s\n", i, h[i]->line);
			i++;
		}
	}
}

