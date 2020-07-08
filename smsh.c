#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#define MAX_LEN 1024 /* The maximum length command */
#define MAX_HISTORY 1024 /* the maximum number of history of commands*/

//global variables
int background = 0;
char cm_list[MAX_LEN][MAX_LEN] = {0,}; // Command string
char history_list[MAX_LEN][MAX_HISTORY] ={0,};
int history_size = 0;
int history_index = 0;
int noclobber_flag = 0;
int path_back_flag = 0;

//function
void  my_set_noclobber(char **arg,int i);
int my_cd(char **arg,int i);
int my_history(char *input);
int my_history_insert(char *input);
char *history_com_return(int i_input);
int check_background(char **cmd,int k);
void background_exit_check(int signo);
int check_redirect_type(char* cm_list_mem,int j);
char* check_redirect(char* cm_list_mem,int type);
int exec_com(char *cm_list_mem,int j);
void each_com_execute(char **args,int i,int j);

void  my_set_noclobber(char **arg,int i){
	if(i==2){
		if(strcmp(arg[1],"+C")==0){
			noclobber_flag = 0;

		}
		else if(strcmp(arg[1],"-C")==0){
			noclobber_flag = 1;

		}
		else{
			printf("Usage : set [+|-]o noclobber\n");
			printf("Usage : set [+|-]C\n");
		}

	}
	else if(i==3){
		if((strcmp(arg[1],"+o")==0)&&(strcmp(arg[2],"noclobber")==0)){
			noclobber_flag = 0;
		}
		else if((strcmp(arg[1],"-o")==0)&&(strcmp(arg[2],"noclobber")==0)){
			noclobber_flag = 1;
		}
		else{
			printf("Usage : set [+|-]o noclobber\n");
			printf("Usage : set [+|-]C\n");
		}
	}
	else{
		printf("Usage : set [+|-]o noclobber\n");
		printf("Usage : set [+|-]C\n");
		
	}
}

int my_cd(char **arg,int i){
	if(i == 1){
		chdir(getenv("HOME"));
	}
	else if(i == 2){
		if(chdir(arg[1])<0)
			printf("FAIL : NO SUCH FILE OR DIRECTORY\n");
	}
	else
		printf("Usage: cd [dirname]\n");
	return 0;
}
int my_history(char *input){
	int s_point; 
	if(history_size > MAX_HISTORY){
		s_point = history_size - (MAX_HISTORY - 1);
		for(int i = 0 ; i< history_index; i++){
			printf(" %6d   %s",s_point++,history_list[i]);
		}
	}

	else{
		for(int i = 0 ; i< history_index; i++){
			printf(" %6d   %s",i+1,history_list[i]);
		}
	}
	
	return 0;
}
int my_history_insert(char *input){
	history_size++;
	if(history_index >= MAX_HISTORY){
		for(int o = 0; o<MAX_HISTORY-1; o++){
			strcpy(history_list[o],history_list[o+1]);
		}
		history_index = MAX_HISTORY-1;
		strcpy(history_list[history_index++],input);
		return 0;
	}

	strcpy(history_list[history_index++],input);
	return 0;
}
char *history_com_return(int i_input){
	if (i_input==0){
		printf("Usage ![history number]\n");
		return 0;
	}
	else if(i_input<0 | i_input>history_size){
		printf("%d: command not found\n",i_input);
		return 0;
	}
	else{
		return history_list[i_input-1];
	}

}
int check_background(char **cmd,int k){
	 // type[0] : 1: true :0 false type[1] : position of &
	int cur_background=0;
	char tmp[MAX_LEN];
	for(int i=0;i<k;i++){
		if(strcmp(cmd[i],"&")==0){
			strcpy(cmd[i],"");
			background++;
			cur_background = 1;
			break;
		}
		for(int p=0; p<strlen(cmd[i]);p++){
			if(cmd[i][p]=='&'){
				background++;
				cur_background = 1;
				strtok(cmd[i],"&");
				break;
			}
		}
	}
	return cur_background;;
}
void background_exit_check(int signo){
	int status,bpid;
	if(background>0){
    		bpid = waitpid(-1,&status,WNOHANG);
    		if(bpid<0){
    		//	perror("waitpid error");
    		}
    		else if(bpid>0){
    			background--;
    			printf("[pid :%d]- background completed\n",bpid);

    			if(WIFEXITED(status)){
    		//		printf("wait : background child normal terminated\n");
    			}
    			else if(WIFSIGNALED(status)){
    		//		printf("wait : background child abnormal terminated\n");
    			}
    		}
    }	
}
int check_redirect_type(char* cm_list_mem,int j){
	char cmp_list_mem[MAX_LEN];

	for(int k = 0; k < strlen(cm_list_mem) ; k++){
	
		switch(cm_list_mem[k])
		{
			case '>':
				if(cm_list_mem[k+1]=='|'){
					//>|
					if(--j!=0)
						return 5;
					return 3;
				}
				if(--j!=0){
					return 1;
				}
				if(cm_list_mem[k+1]=='>'){
					//'>>'
					return 1;
				}
				else{//'>'
					return 0;
				}
				break;
			case '<'://'<'
				return 2;
				break;
			default:
				break;
		}
	}
	return 4;

}
char* check_redirect(char* cm_list_mem,int type){ //actual redirect working in here
	int fd;
	char temp_cm_mem[MAX_LEN];
	char *result;
	char *types[6] = {">",">>","<",">|","",">|"};
	strcpy(temp_cm_mem,cm_list_mem);
	
	
	if(type ==4) return cm_list_mem;
	if(type==1 | type == 0 ) result = strtok(temp_cm_mem,types[0]);
	else if(type == 3 | type == 5){ result = strtok(temp_cm_mem,types[type]);}
	else{result = strtok(temp_cm_mem,"<");}
	switch(type)
	{
		case 0:/*>*/
			result = strtok(NULL,types[type]);
			result = strtok(result," ");
			if(!noclobber_flag){
				if((fd = open(result,O_CREAT|O_WRONLY|O_TRUNC,0644))<0)
					perror("file open error");
				dup2(fd,1);
				close(fd);
			}
			else{
				printf("smsh: %s: cannot overwrite existing file\n",result);
				exit(1);
			}
			break;
		case 1:/*">>"*/
			result = strtok(NULL,types[type]);
			result = strtok(result," ");
			if((fd = open(result,O_CREAT|O_APPEND|O_WRONLY,0644))<0)perror("file open error");
			dup2(fd,1);
			close(fd);
			break;
		case 2:/*<*/
			result = strtok(NULL,"<");
			result = strtok(result," ");
			if((fd = open(result,O_CREAT|O_RDONLY,0644))<0)perror("file open error");
			dup2(fd,0);
			close(fd);
			break;
		case 3:
			result = strtok(NULL,types[type]);
			result = strtok(result," ");
			if((fd = open(result,O_CREAT|O_WRONLY|O_TRUNC,0644))<0)
				perror("file open error");
			dup2(fd,1);
			close(fd);
			break;
		case 4://no redirection
			break;
		case 5:
			result = strtok(NULL,types[type]);
			result = strtok(result," ");
			if((fd = open(result,O_CREAT|O_WRONLY|O_APPEND,0644))<0)
				perror("file open error");
			dup2(fd,1);
			close(fd);
			break;
	}
	return strtok(temp_cm_mem,types[type]);
}
int exec_com(char *cm_list_mem,int j){
	sigset_t unlock;
	sigemptyset(&unlock);
	sigaddset(&unlock,SIGINT);
    sigprocmask(SIG_UNBLOCK,&unlock,NULL);
	
	char *cmd_mem[MAX_LEN / 2 + 1]; // command line arguments
	char *cmd_no_redir[MAX_LEN / 2 + 1];
	char *tmp_cmd_mem;
	int i = 0;
	int quoat_num = 0;
	//check redirect and change stdout or stdin flow
	//printf("cmd : [%s]\n",cm_list_mem);
	int type = check_redirect_type(cm_list_mem,j);
	//printf("type : %d , j :%d\n",type,j);
    strcpy(cm_list_mem,check_redirect(cm_list_mem,type));
	cmd_mem[i] = strtok(cm_list_mem," ");
  	while((cmd_mem[++i]=strtok(NULL," "))!=NULL);
  	cmd_mem[i] = NULL;
  	

  	i = 0 ;
	
  	while(cmd_mem[i] !=NULL){
  		if(cmd_mem[i][0] == '\"' && cmd_mem[i][strlen(cmd_mem[i])-1] == '\"' ){
			quoat_num = strlen(cmd_mem[i]);
			tmp_cmd_mem = strtok(cmd_mem[i],"\"");
			strcpy(cmd_mem[i],tmp_cmd_mem);
			if(quoat_num > strlen(cmd_mem[i]) + 2){
				perror("wrong quoation usage");
				exit(1);
			}
		}
		i++;
  	}
  	// printf("cmd_mem : %s\n",cmd_mem[1]);

  	execvp(cmd_mem[0],cmd_mem);
  	perror("error exec");
  	exit(1);
	return -1;
}
void each_com_execute(char **args,int i,int j){

	int fd[2]; // fd array for pipe
	int q = 0;
	int o = 0;
	int pid;
	int pipe_count = 0;
	int pre_pipe_add = 0;
	char temp_command_set[MAX_LEN] = "";
	char *result;
	char store[MAX_LEN] ={0,};
	
	for(int s = 0; s<i ; s++){
		strcat(temp_command_set, args[s]);
		if(s!=i-1) strcat(temp_command_set, " ");
	}
	if(strchr(temp_command_set,'|')!=NULL){
		//printf("hello\n");
		result=strtok(temp_command_set,"|");
		strcpy(store,result);
		if(store[strlen(store)-1]=='>'){
			result = strtok(NULL,"|");
			strcat(store,"|");
			strcat(store,result);	
		}
		else{
			pipe_count++;
			pre_pipe_add++;
		}
		//printf("store: [%s]\n",store);
		strcpy(cm_list[q++],store);
		while((result=strtok(NULL,"|"))!=NULL){
			strcpy(store,result);
			if(store[strlen(store)-1]=='>'){
				result = strtok(NULL,"|");
				strcat(store,"|");
				strcat(store,result);
			}
			else{
				if(pre_pipe_add==1){
					pre_pipe_add--;
				}
				else{
					pipe_count++;
				}
			}
			strcpy(cm_list[q],store);
			if(++q==strlen(temp_command_set))
				break;
		}
	}
	else{
		strcpy(cm_list[o],temp_command_set);
	}
		
	int test =0;

	for(o = 0; o< pipe_count;o++){
		pipe(fd);
		if((pid=fork())<0)
			perror("fork error");
		if(pid==0){
			dup2(fd[1],1); // STDOUTPUT TO PIPE OUTPUT
			close(fd[0]);
			exec_com(cm_list[o],j);
		}	
		else{
			dup2(fd[0],0);
			close(fd[1]);
			wait(NULL);
		}
	}
	exec_com(cm_list[o],j);
}
void nothing(void){
}
int main(void) {
  char *args[MAX_LEN / 2 + 1]; // command line arguments
  char *input; // command input by user
  char *temp_input; //copy of input for multiple strtok use
  char tmp[MAX_LEN]; // temp for strtok
  char tmp2[MAX_LEN][MAX_LEN]; //temp for strtok
  char *cm; //command executing
 // char cm_queue[MAX_LEN][MAX_LEN]; //  queue for multiple command seperated by ";"
  char *temp_cm; // temp cm for strtok
  char *history_com; // COMMAND SAVED AT HISTORY
  int should_run = 1;          /* flag to determine when to exit program */
  int cm_count = 0; // the number of commands requested by ";"
  int pid,npid,status; 
  int i,j;
  int type;
//  printf("QUIT COMMAND : [exit]\n");
//  printf("------------------------------------\n");
  while(should_run){
  	int i;
	sigset_t mask;
	
	sigfillset(&mask);
	sigdelset(&mask,SIGCHLD);
	sigdelset(&mask,SIGINT);
	sigprocmask(SIG_SETMASK,&mask,NULL);
	signal(SIGCHLD,background_exit_check);// for background child
 
  
  	input = calloc(MAX_LEN,sizeof(char));
  	temp_input = calloc(MAX_LEN,sizeof(char));
  	cm = calloc(MAX_LEN,sizeof(char));
  	cm_count = 0;
  	//shell command input format
	
    printf("[%s]:$ ",getcwd(NULL,0));

    fflush(stdout);
    fgets(input,MAX_LEN, stdin);
    /* strcpy,strtok for !number command */
    strcpy(temp_input,input);
    strtok(temp_input," ");
    //command is empty
	// printf("input[%s]\n",input);
  	if(strcmp(input,"\n")==0){
  		goto empty_command;
  	}
	else{
		if(temp_input[0]=='!'){
			if(strlen(temp_input)<2){
				strcpy(temp_input,input);
				strtok(temp_input,"\n");
				printf("%s: command not found\n",temp_input);
				my_history_insert(input);
				goto empty_command;
			}
			if((history_com=history_com_return(atoi(strtok(temp_input,"!"))))!=0){
				input=history_com;
				// strcpy(input);
			}
			else{
				goto empty_command;
			}
		}
		my_history_insert(input);

	} 

 	//check and count semicolumn
  	strcpy(temp_input,input);
  	
  	strcpy(temp_input,input);
  	//start seperate command by ;
  	char cm_queue[MAX_LEN][MAX_LEN] = {0,};
	char is_path[MAX_LEN] = {0,};
	int left = 0;
	int right = 0;
	int cm_queue_i  = 0;
	for(int i = 0; i<strlen(input); i++){
		if(input[i] == '('){
			left++;
		}
		else if(input[i] == ')'){
			right++;
		}
		if(left == 1 && right == 1){
			if(input[i] == ';'){
				strncpy(cm_queue[cm_queue_i++],input,i);
				is_path[cm_queue_i-1] = 1;
				input += i+1;
				left = 0;
				right = 0;
				i = -1;
			}
			else if(i == strlen(input)-1){
				strncpy(cm_queue[cm_queue_i++],input,i);
				is_path[cm_queue_i-1] = 1;

			}
		}
		else if(left==0 &&right ==0 && input[i] == ';'){
			strncpy(cm_queue[cm_queue_i++],input,i);
			int cm_queue_c = 0;
			for(int b = 0 ; b < strlen(cm_queue[cm_queue_i-1]);b++){
				if(cm_queue[cm_queue_i-1][b]==' '){
					cm_queue_c++;
				}
				if(cm_queue_c == strlen(cm_queue[cm_queue_i-1])){
					cm_queue_i--;
				}
			}
			input += i+1;
			i = -1;
		}
		else if((left==0 &&right ==0) &&(i == strlen(input)-1)){
			strncpy(cm_queue[cm_queue_i++],input,i);

		}
		
	}

	cm_count =cm_queue_i;
	//end of command array
  	j = 0;
  	i = 0;
  	//loop for excute number of ;
  	while(j<cm_count){
  		if(is_path[j]==1){//(command)
		  	char  abc[MAX_LEN][MAX_LEN]={0,};
			char *path_result;
			char store[MAX_LEN];
			char path_command[MAX_LEN]={0,};
			char path_cmd_list[MAX_LEN][MAX_LEN]={0,};
			int i=0;
			int path_cm_count = 0;
			int back_index;
			char path_cm_queue[MAX_LEN][MAX_LEN]={0,};
			path_back_flag=0;

			strcpy(abc[i++],strtok(cm_queue[j++],"()"));
		
			while((path_result = strtok(NULL,"()"))!=NULL){
				strcpy(abc[i++],path_result);
					// path_back_flag++;
			}
			if(i==1){
		
				strcpy(path_command,abc[0]);
				path_result = strtok(path_command,";");
			  	while(1){
			  		path_cm_count++;
			  		if((path_result=strtok(NULL,";"))==NULL){
			  			break;
			  		}
			  	}
			  	strcpy(path_command,abc[0]);
			  	path_result = strtok(path_command,";");
				i=0;
			  	while(1){
			  		strcpy(store,path_result);
			  		strcpy(path_cm_queue[i++],store);
			  		if((path_result=strtok(NULL,";\n"))== NULL)break;
	  			}
				
			}
			else if(i==3){
				if((back_index = strcspn(abc[2],"&"))!=strlen(abc[2])){
					path_back_flag++;
					background++;
					if(back_index==0){
						strcpy(abc[1],"");
					}
					else{
						strtok(abc[1],"&");
					}
				}
					strcpy(path_command,abc[1]);
				path_result = strtok(path_command,";");
			  	while(1){
			  		path_cm_count++;
			  		if((path_result=strtok(NULL,";"))==NULL){
			  			break;
			  		}
			  	}
			  	strcpy(path_command,abc[1]);
			  	path_result = strtok(path_command,";");
				i=0;
			  	while(1){
			  		strcpy(store,path_result);
			  		strcat(store,abc[2]);
			  		strcpy(path_cm_queue[i++],store);
			  		if((path_result=strtok(NULL,";\n"))== NULL)break;
	  			}
					
			}//i==3
			else if(i==2){
				if((back_index = strcspn(abc[1],"&"))!=strlen(abc[1])){
					path_back_flag++;
					background++;
					if(back_index==0){
						strcpy(abc[1],"");
					}
					else{
						strtok(abc[1],"&");
					}
				}
					strcpy(path_command,abc[0]);
				path_result = strtok(path_command,";");
			  	while(1){
			  		path_cm_count++;
			  		if((path_result=strtok(NULL,";"))==NULL){
			  			break;
			  		}
			  	}
			  	strcpy(path_command,abc[0]);
			  	path_result = strtok(path_command,";");
				i=0;
			  	while(1){
			  		strcpy(store,path_result);
			  		strcat(store,abc[1]);
			  		strcpy(path_cm_queue[i++],store);
			  		if((path_result=strtok(NULL,";\n"))== NULL)break;
		 		}
		
			}//i==2
			if((pid=fork())<0){
				perror("fork error");
				exit(1);
			}
			else if(pid==0){
				int j=0;int i=0;
				while(j<path_cm_count){	
			  		temp_cm = calloc(MAX_LEN,sizeof(char));
			  		for(int p = 0; p<i-1;p++){
			  			args[p] = NULL;
			  		}
			  		i = 0;
			  		args[i] = strtok(path_cm_queue[j++]," ");
			  		while((args[++i]=strtok(NULL," "))!=NULL);
			  		args[i] = NULL;
			  		
					if((pid =fork())<0){
						perror("fork error");
						goto path_wrong_way;
					}
					else if(pid ==0){//child
						each_com_execute(args,i,j);
					}
					else{//parent
						if(type){
			    			printf("[pid :%d]+ background excuting\n",pid);
			    			break;
						}
			            wait(&status);
					}
					//end of type 0 or 6
					path_wrong_way:
					nothing();
			  		//free(temp_cm);
			  	}
			  	exit(0);
			}
			else{
				if(path_back_flag){
	    			printf("[pid :%d]+ background excuting\n",pid);

					break;
				}
				else{
					if(wait(&status)<0){
						perror("child wait error");
						//break;
					}
				}

			}
  		}


  		else{//no patheness command
  			if(strlen(cm_queue[j])==0){
  				j++;
  				goto wrong_way;
  			}
	  		temp_cm = calloc(MAX_LEN,sizeof(char));
	  		for(int p = 0; p<i-1;p++){
	  			args[p] = NULL;
	  		}
	  		i = 0;

	  		args[i] = strtok(cm_queue[j++]," ");
	  		while((args[++i]=strtok(NULL," "))!=NULL);
	  		args[i] = NULL;

	  		//chand directory command or exit command (shell built in command)
	  		if(strcmp(args[0],"cd")==0){
	  			my_cd(args,i);
	  			goto wrong_way;
	  		}
	  		if(strcmp(args[0],"exit")==0){
	  			exit(0);
	  		}
	  		if(strcmp(args[0],"history")==0){
	  			my_history(input);
	  			goto wrong_way;
	  		}
	  		if(strcmp(args[0],"set")==0){
	  			my_set_noclobber(args,i);
	  			goto wrong_way;
	  		}
	  		

	  		//external program execution command
	  		type=check_background(args,i);
			//type=6,type=0 start
			if((pid =fork())<0){
				perror("fork error");
				goto wrong_way;
			}
			else if(pid ==0){//child
				each_com_execute(args,i,1);
			}
			else{//parent
				if(type){
	    			printf("[pid :%d]+ background excuting\n",pid);
	    			break;
				}
	            wait(&status);
			}
		}
		//end of type 0 or 6
		wrong_way:
		nothing();
  		//free(temp_cm);
  	}
  	//end of ; loop

  	empty_command:
  	nothing();
  	
  }
  //end of shell loop
  return 0;
}
