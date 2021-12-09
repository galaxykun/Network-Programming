#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<netdb.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<pthread.h>
#include<signal.h>


#define BACKLOG 50
#define PORT 7777


int serverSocket;
int online[100];
int gamming[100];
int size = 100;
char* account[105];
pthread_mutex_t online_mutex = PTHREAD_MUTEX_INITIALIZER; // pthread 互斥鎖
pthread_mutex_t passwd_mutex = PTHREAD_MUTEX_INITIALIZER; // pthread 互斥鎖
void *gamemenu();



static void sig_handler(int sig)
{
	char command[32];
	if(sig==SIGINT){
		for(int i=0; i<10; i++){
			if(online[i]==1) send(i,"Close ",strlen("Close "),0);;
		}
		usleep(1000);
		close(serverSocket);
		exit(0);
	}
}

int authe(char buf[])
{
    pthread_mutex_lock(&passwd_mutex);
	FILE *fp;
	char tmp1[100];
	fp=fopen("passwd","r");
	while(fscanf(fp,"%s",tmp1)!=EOF)
	{
		if(strcmp(tmp1,buf)==0){
            return 1;
        }
	}
    pthread_mutex_unlock(&passwd_mutex);
	return 0;
}

void init(){

	struct sockaddr_in serverAddress;

	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(serverSocket<0){
		fprintf(stderr, "Error: socket\n");
		exit(EXIT_FAILURE);
	}
	
	bzero(&serverAddress, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(PORT);
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
		fprintf(stderr, "Error: bind\n");
		exit(EXIT_FAILURE);
	}

	if(listen(serverSocket, BACKLOG) < 0){
		fprintf(stderr, "Error: listen\n");
		exit(EXIT_FAILURE);
	}

    for(int i=0; i<105; i++)
		account[i] = (char *)malloc(sizeof(char)*128);
}

void getAlluser(int fd){

	char bar[100]="\n--------------------\n";
	printf("%d user.\n", fd);
	send(fd,bar,strlen(bar),0);
	send(fd,"[目前在線名單]\n",strlen("[目前在線名單]\n"),0);
	for (int i = 0;i < 100; i++){
		if (online[i] != 0){
			char buf[100] = {};
			if(i != fd){
				sprintf(buf, "user: [%s] fd:%d\n" ,account[i] ,i);
				send(fd,buf,strlen(buf),0);
			}
		}
	}
	send(fd,bar,strlen(bar),0);
	char buf[256] = {};
	strcpy(buf,"請選擇你的對手: (enter @fd) 或輸入其他指令");
	send(fd,buf,strlen(buf),0);
}

void SendMsgToAll(char* msg){
	int i;
	for (i = 0;i < size; i++){
		if (online[i] != 0){
			printf("System sending to %d\n",i);
            //printf("i is: %d\n",i);
			send(i,msg,strlen(msg),0);
		}
	}
}


int main(){

    int clientSocket;

    // 信號終止處理
	struct sigaction sa;
    sa.sa_handler = sig_handler;
	sigemptyset(&sa.sa_mask); //該函數的作用是將信號集初始化為空
	sa.sa_flags = SA_RESTART; //被信号中断的系统调用会自行重启

	if(sigaction(SIGINT, &sa, NULL)==-1){
		fprintf(stderr, "Error: sigaction\n");
		exit(EXIT_FAILURE);
	}

    init();
	printf("Server Initial Successful!\n");

    pthread_t t; //宣告 pthread 變數
	pthread_attr_t attr; 
	int count=0;

    // 初始化
	memset(online, 0, sizeof(online));
	memset(gamming, 0, sizeof(gamming));
	pthread_attr_init(&attr);                                       // thread detached 對執行緒屬性變數的初始化
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);	// 設定執行緒 detachstate 屬性  新執行緒不能用pthread_join()來同步

    while(1){
		clientSocket = accept(serverSocket, NULL, NULL); //printf(">>>%d\n",clientSocket);
        if (clientSocket == -1){
			printf("Client occurr Error ...\n");
			continue;
		}
        for(int i = 0; i < size; i++)
            if(size == i){
            char* str = "Sorry, the room is full!! Please try again!!";
            send(clientSocket,str,strlen(str),0);
            close(clientSocket);  
        }
        
		pthread_create(&t, &attr, gamemenu, (void *)&clientSocket); // 建立子執行緒
		pthread_mutex_lock(&online_mutex);
		online[clientSocket]=1;
		pthread_mutex_unlock(&online_mutex);
	}
	pthread_mutex_destroy(&online_mutex);
	close(serverSocket);

    return 0;
}


void *gamemenu(void *arg){

    int clientSocket = *(int *)arg;
    char buffer[512];
    char *ptr;
    char tmp[100];

    printf(">>>>>clientSocket: %d\n", clientSocket);

    while(1){
        memset(buffer, 0, sizeof(buffer));
        send(clientSocket ,"authenticate",strlen("authenticate"),0);
        recv(clientSocket,buffer,sizeof(buffer),0);//收到帳密
        printf("buf=%s\n",buffer);
        ptr=strstr(buffer,":");
        *ptr='\0';
        strcpy(account[clientSocket], buffer);
        account[clientSocket][strlen(account[clientSocket])] = '\0';
        *ptr=':';
        ptr=strstr(buffer,"@");
        *ptr='\0';
        char tmp[128];
        if(authe(buffer)){
            pthread_mutex_unlock(&passwd_mutex);
			ptr=strstr(buffer,":");
			*ptr='\0';
			printf("New account : %s\n",  account[clientSocket]);
            memset(tmp, 0, sizeof(tmp));
            sprintf(tmp, "Successfully %d",clientSocket);
            send(clientSocket,tmp,strlen(tmp),0);
            memset(tmp, 0, sizeof(tmp));
            strcpy(tmp, account[clientSocket]);
            strcat(tmp, " is join!!\n");
			SendMsgToAll(tmp);       
			break;
		}
		else{
            memset(tmp, 0, sizeof(tmp));
            strcpy(tmp, "Fail");
            send(clientSocket,tmp,strlen(tmp),0);
            printf(">>%s\n",tmp);
        }
    }
    while(1){
        memset(buffer, 0, sizeof(buffer));
        if (recv(clientSocket,buffer,sizeof(buffer),0) <= 0){
			// 對方關閉了
			int i;
            for (i = 0;i < size; i++){
				if (clientSocket == i){
					online[i] = 0; //設為這個client不存在了
					break;
				}
			}
			printf("退出：fd = %d quit\n",clientSocket);
			// char tmp[128] = ;
			// strcpy(tmp, "Leave ");
			// strcat(tmp,)
			// SendMsgToAll(fprintf("Leave %d ",clientSocket));
			pthread_exit((void*)i);
		}

        if (strcmp(buffer,"ls") == 0){
			getAlluser(clientSocket);
		}
        else if(buffer[0]=='@')
		{
			int oppofd=atoi(&buffer[1]);
			char *msg = (char*)malloc( 256*sizeof(char) );
			strcpy(msg,"CONNECT ");
			strcat(msg,account[clientSocket]);
			sprintf(tmp," %d",clientSocket);
			strcat(msg,tmp);
			if(oppofd == clientSocket){
				send(clientSocket,"不能挑戰自己!!!\n<<<請重新輸入指令>>>\n\0",strlen("不能挑戰自己!!!\n<<<請重新輸入指令>>>\n\0"),0);
				continue;
			}
			else if(gamming[oppofd] == 1)
				send(clientSocket,"遊戲中...無法接受邀請!!\n\0",strlen("遊戲中...無法接受邀請!!\n\0"),0); 
			else{
				send(oppofd,msg,strlen(msg),0);
				send(clientSocket,"等待對方的回應...\n\0",strlen("等待對方的回應...\n\0"),0);
			}
		}
		else if(strncmp(buffer,"AGREE ",6)==0)
		{
			int oppofd=atoi(&buffer[6]);
			char *msg = (char*)malloc( 256*sizeof(char) );
			strcpy(msg,"AGREE ");
			strcat(msg,account[clientSocket]);
			sprintf(tmp," %d",clientSocket);
			strcat(msg,tmp);
			send(oppofd,msg,strlen(msg),0);
		}
        else if (strncmp(buffer,"Reject ",7)==0){
            int oppofd=atoi(&buffer[7]);
			char *msg = (char*)malloc( 256*sizeof(char) );
			strcpy(msg,"Reject ");
			strcat(msg,account[clientSocket]);
			sprintf(tmp," %d",clientSocket);
			strcat(msg,tmp);
			send(oppofd,msg,strlen(msg),0);
        }
        else if (strncmp(buffer,"Leave ",6)==0){
            int oppofd=atoi(&buffer[6]);
			char *msg = (char*)malloc( 256*sizeof(char) );
			strcpy(msg,"Leave ");
			strcat(msg,account[clientSocket]);
			sprintf(tmp," %d",clientSocket);
			strcat(msg,tmp);
			send(oppofd,msg,strlen(msg),0);
        }
		else if (strncmp(buffer,"Gammingstart",12)==0){
            gamming[clientSocket] = 1;
        }
		else if (strncmp(buffer,"Gammingend",10)==0){
            gamming[clientSocket] = 0;
        }
        else if(buffer[0]=='#')
		{
			int n=atoi(&buffer[1]),oppofd;
			char *ptr,tmp[100];
			ptr=strstr(buffer," ");
			ptr++;
			oppofd=atoi(ptr);
			
				sprintf(tmp,"#%d",n);
				printf("[%d] buf=%s\n",clientSocket,tmp);
				send(oppofd,tmp,strlen(tmp),0);
			
		}
		// else if(strncmp(buffer,"Gamming ",8)==0){
		// 	int fd=atoi(&buffer[9]);
		// 	send(fd,"遊戲中...無法接受邀請!!\n\0",strlen("遊戲中...無法接受邀請!!\n\0"),0);
		// }
		else{
			SendMsgToAll(buffer);
		}
    }
}