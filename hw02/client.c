#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<signal.h>
#include<fcntl.h>
#include<errno.h>
#include<pthread.h>
#define SERVERPORT 7777 


int sockfd;
int myfd;
int yes_no = 0;
char sendbuf[128];
char recvbuf[128];
// int f = 1;
char name[30];
char passwd[30];
int oppofd, G=0, IsMe=0;
char opponame[100],le1='O',le2='X', x[9];
pthread_mutex_t data_mutex = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t match_mutex = PTHREAD_MUTEX_INITIALIZER;
void *sendsock(void *arg);
void *recvsock(void *arg);
int start();
void map();
int iswin(char le);
int istie();

static void sig_handler(int sig)
{
	char command[128];
	memset(command, 0, sizeof(command));
	if(sig==SIGINT){
		char tmp[128];
		sprintf(tmp,"Leave %d",oppofd);
		send(sockfd,tmp,strlen(tmp),0);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"[INFO] %s 退出了聊天室",name);
		send(sockfd,tmp,strlen(tmp),0);
		printf("\n\t<<<Close the program>>>\n\n");
		close(sockfd);
		exit(0);
	}
}

void init(){
    int connectok;
	struct sockaddr_in serverAddress;
	const char *serverIP = "127.0.0.1";
	unsigned short serverPort = SERVERPORT;
	struct sigaction sa;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0){
		fprintf(stderr, "Error: socket\n");
		exit(EXIT_FAILURE);
	}

	memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr(serverIP);
	serverAddress.sin_port = htons(serverPort);

	sa.sa_handler = sig_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;

	connectok = connect(sockfd, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
	if(sigaction(SIGINT, &sa, NULL)==-1){
		fprintf(stderr, "Error: sigaction\n");
		exit(EXIT_FAILURE);
	}
}

int start(){
    char buffer[128];
    while(1){
        memset(buffer, 0, sizeof(buffer));
		recv(sockfd,buffer,sizeof(buffer),0);
		if (strncmp(buffer,"authenticate",12) == 0){
			send(sockfd,name,strlen(name),0);
		}
        else if (strncmp(buffer,"Successfully",12) == 0){
            printf("[INFO] Client start Successfully! \n<<< Welcome!! >>>\n");
			char *ptr;
			ptr=strstr(buffer," ");
			ptr++;
			myfd = atoi(ptr);
			// printf("MYFD is: %d\n",myfd);
            printmenu();
            break;
        }
        else if (strncmp(buffer,"Fail", 4) == 0){
            printf("[INFO] Account or password incorrect! \n");
            printf("[INFO] 請輸入帳號 :");
            scanf("%s",name);
            printf("[INFO] 請輸入密碼 :");
            scanf("%s",passwd);
            strcat(name,":");
            strcat(name,passwd);
            strcat(name,"@");
        }
	}
    char *ptr;
	ptr=strstr(name,":");
	*ptr='\0';

    pthread_t recvsock_t, sendsock_t;

    pthread_create(&recvsock_t, NULL, recvsock, (void*)&sockfd);
	pthread_create(&sendsock_t, NULL, sendsock, (void*)&sockfd);
	pthread_join(recvsock_t, NULL);

	pthread_mutex_destroy(&data_mutex);

	close(sockfd);
	return 0;

}

void *recvsock(void *arg){
    char *ptr,*qtr;
	while(1){
        
        memset(recvbuf, 0, sizeof(recvbuf));
		if (recv(sockfd,recvbuf,sizeof(recvbuf),0) <= 0){
			return;
		}
		pthread_mutex_lock(&data_mutex);
		if(strncmp(recvbuf,"CONNECT ",8)==0)
		{
			ptr=strstr(recvbuf," ");
			ptr++;
			qtr=strstr(ptr," ");
			*qtr='\0';
			strcpy(opponame,ptr);
			qtr++;
			if(G == 1){
				// int fd=atoi(qtr);
				// char tmp[256];
				// sprintf(tmp,"Gamming %d",atoi(qtr));
				// send(fd,"遊戲中...無法接受邀請!!\n\0",strlen("遊戲中...無法接受邀請!!\n\0"),0);//tmp,strlen(tmp),0);
				// printf(">>>here\n");
			}
			else{
				oppofd=atoi(qtr);
				printf("[INFO]Do you agree to start a new game with [%s] (yes/no)?\n  ",opponame);
				yes_no = 1;
				
				// while (1)
				// {
				// 	pthread_mutex_unlock(&data_mutex);
					
				// 	// memset(buf, 0, sizeof(buf));
				// 	// fgets(buf, sizeof(buf), stdin);
				// 	printf("%s\n",sendbuf);
				// 	if(strcmp(sendbuf,"yes") == 0)
				// 	{
				// 		printf("[INFO]connect sucessful\n");
				// 		sprintf(sendbuf,"AGREE %d",oppofd);
				// 		send(sockfd,sendbuf,strlen(sendbuf),0);
				// 		printf("[INFO]Game Start!\n");
				// 		int i;
				// 		for(i=0;i<9;i++)x[i]=' ';
				// 		le1='O';le2='X';
				// 		G=1;
				// 		if(le1=='O')IsMe=1;
				// 		else IsMe=0;
				// 		map();
				// 		if(IsMe)printf("Please enter #(1-9) or enter -1 to leave the game\n");
				// 		else printf("Wait for your opponent....\n");
				// 		yes_no = 0;
				// 		break;
						
				// 	}
				// 	else if(strcmp(sendbuf,"no") == 0){
				// 		printf("[INFO]Reject !!\n");
				// 		sprintf(sendbuf,"Reject %d",oppofd);
				// 		send(sockfd,sendbuf,strlen(sendbuf),0);
				// 		yes_no = 0;
				// 		break;
				// 	}
				// 	// else{
				// 	// 	printf("Please enter yes/no !!\n");
				// 	// }
				// 	usleep(10000);
				// 	pthread_mutex_lock(&data_mutex);
				// }
				
			}
		}
		else if(strncmp(recvbuf,"AGREE ",6) == 0)
		{
			ptr=strstr(recvbuf," ");
			ptr++;
			qtr=strstr(ptr," ");
			*qtr='\0';
			strcpy(opponame,ptr);
			qtr++;
			oppofd=atoi(qtr);
			printf("[INFO]%s agree with you.\n",opponame);
			printf("[INFO]Game Start!\n");
			int i;
			for(i=0;i<9;i++)x[i]=' ';
			le1='X';le2='O';
			G=1;
			send(sockfd,"Gammingstart",strlen("Gammingstart"),0);
			if(le1=='O')IsMe=1;
			else IsMe=0;
			map();
			if(IsMe)printf("Please enter #(1-9) or enter -1 to leave the game\n");
			else printf("Wait for your opponent....\n");
		}
        else if(strncmp(recvbuf,"Reject ",7) == 0){
            ptr=strstr(recvbuf," ");
			ptr++;
			qtr=strstr(ptr," ");
			*qtr='\0';
			strcpy(opponame,ptr);
			qtr++;
			oppofd = 0;
			printf("[INFO]%s reject you.\n",opponame);
            printf("[INFO]Please re-enter!\n");
        }
        else if(strncmp(recvbuf,"Leave ",6) == 0){
				
				ptr=strstr(recvbuf," ");
				ptr++;
				qtr=strstr(ptr," ");
				*qtr='\0';
				strcpy(opponame,ptr);
				qtr++;
				oppofd = 0;
				printf("[INFO]%s leave the gmae.\n",opponame);
				printf("[INFO]Please re-enter the command.\n");
				G = 0;
				send(sockfd,"Gammingend",strlen("Gammingend"),0);
				IsMe = 0;
				// f = 0;
			
        }
        else if(strncmp(recvbuf,"Close ",6) == 0){
            printf("[INFO]Server is close.\n");
            printf("[INFO]Please try to connect later.\n");
            printf("\n\t<<<Close the program>>>\n\n");
            break;
        }
		else if(recvbuf[0]=='#')
		{
			x[atoi(&recvbuf[1])]=le2;
			if(iswin(le2))
			{
				printf("[INFO]You lose!\n\n");
				G=0;
				send(sockfd,"Gammingend",strlen("Gammingend"),0);
			}
			else if(istie())
			{
				printf("[INFO]It's a tie!\n\n");
				G = 0;
				send(sockfd,"Gammingend",strlen("Gammingend"),0);
				map();
			}
			else{
				IsMe=1;
				map();
				printf("Please enter #(1-9) or enter -1 to leave the game\n");
			}
		}
		else{
			printf("%s\n",recvbuf);
		}

        pthread_mutex_unlock(&data_mutex);
		usleep(1000);
	}
}

void *sendsock(void *arg){
    pthread_detach(pthread_self());
	while(1){
		// pthread_mutex_lock(&data_mutex);
		char buf2[128];
        memset(sendbuf, 0, sizeof(sendbuf));
        memset(buf2, 0, sizeof(buf2));
		fgets(sendbuf, sizeof(sendbuf), stdin);
		// pthread_mutex_lock(&data_mutex);
		// if(G==1&&IsMe==1)
		// {
		// 	// map();
		// 	printf("Please enter #(1-9) or enter -1 to leave the game\n");
		// 	// f=0;
		// }
		// // if(G==1)f=0;
		char *ptr = strstr(sendbuf, "\n");
		*ptr = '\0';
		char msg[131] = {};
		if (strcmp(sendbuf,"quit") == 0){
			if(G == 1){
				printf("遊戲中此指令不能使用!!請輸入 -1 !!\n");
				continue;
			}
			memset(buf2,0,sizeof(buf2));
			printf("\n\t<<<Close the program>>>\n\n");
			sprintf(buf2,"[INFO] %s 退出了聊天室",name);
			send(sockfd,buf2,strlen(buf2),0);
			break;
		}
		if (strcmp(sendbuf,"user") == 0){
			memset(buf2,0,sizeof(buf2));
			sprintf(buf2,"ls");
			send(sockfd,buf2,strlen(buf2),0);
		}
		else if(sendbuf[0]=='@')
		{
			if(G == 1){
				printf("你在遊戲中無法邀請其他人!\n");
			}
			else{
				sprintf(msg,"%s",sendbuf);
				send(sockfd,msg,strlen(msg),0);
			}
		}
		else if(sendbuf[0]=='#')
		{
			if(G==0)printf("Game is end or not start.\n");
			else if(IsMe==0)printf("Is not your part.Please wait your opponent.\n");
			else{
				int n=atoi(&sendbuf[1])-1;
				if(x[n]!=' '||n>9||n<0)
				{
					printf("Please enter another number.#(1-9)\n");
				}
				else
				{
					x[n]=le1;
					printf("----------\n");
					map();
					if(iswin(le1))
					{
						printf("[INFO]You win!!\n\n");
						G=0;
						send(sockfd,"Gammingend",strlen("Gammingend"),0);
					}
					else if(istie())
					{
						printf("[INFO]It's a tie\n\n");
						G=0;
						send(sockfd,"Gammingend",strlen("Gammingend"),0);
					}
					else printf("\nWait for your opponent....\n");
					IsMe=0;
					sprintf(msg,"#%d %d",n,oppofd);
					send(sockfd,msg,strlen(msg),0);
				}
			}
		}
		// else if(buf[0]=='#')
		// {
		// 	x[atoi(&buf[1])]=le2;
		// 	if(iswin(le2))
		// 	{
		// 		printf("You lose!\n\n\n\n");
		// 		G=0;
		// 	}
		// 	else if(istie())
		// 	{
		// 		printf("It's a tie!\n\n\n\n");
		// 		G=0;
		// 	}
		// 	IsMe=1;;
		// }
        else if(strcmp(sendbuf,"yes") == 0 && yes_no == 1)
		{
			printf("[INFO]connect sucessful\n");
			sprintf(sendbuf,"AGREE %d",oppofd);
			send(sockfd,sendbuf,strlen(sendbuf),0);
			printf("[INFO]Game Start!\n");
			int i;
			for(i=0;i<9;i++)x[i]=' ';
			le1='O';le2='X';
			G=1;
			send(sockfd,"Gammingstart",strlen("Gammingstart"),0);
			if(le1=='O')IsMe=1;
			else IsMe=0;
			map();
			if(IsMe)printf("Please enter #(1-9) or enter -1 to leave the game\n");
			else printf("Wait for your opponent....\n");
			yes_no = 0;
			
		}
        else if(strcmp(sendbuf,"no") == 0 && yes_no == 1){
            printf("[INFO]Reject !!\n");
            sprintf(sendbuf,"Reject %d",oppofd);
			send(sockfd,sendbuf,strlen(sendbuf),0);
			yes_no = 0;
        }
        // else if(strcmp(sendbuf,"yes") == 0 || strcmp(sendbuf,"no") == 0);
		else if(strcmp(sendbuf,"-1") == 0 && G == 1){
            printf("[INFO]Leave the game !!\n");
            sprintf(sendbuf,"Leave %d",oppofd);
			send(sockfd,sendbuf,strlen(sendbuf),0);
			G = 0;
			send(sockfd,"Gammingend",strlen("Gammingend"),0);
			IsMe = 0;
			// f = 0;
        }
		// else if(strcmp(buf,"print")==0)
		// {
		// 	print();
		// }
        else if(strcmp(sendbuf,"menu")==0)
		{
			printmenu();
		}
		else if(strcmp(sendbuf,"print")==0)
		{
			map();
		}
		else if(sendbuf[0]!='\0'){
			if(yes_no == 1){
				printf("Please enter yes/no !!\n");
				//pthread_mutex_unlock(&data_mutex);
				continue;
			}
			sprintf(msg,"[%s] : %s",name,sendbuf);
			send(sockfd,msg,strlen(msg),0);
		}
		// pthread_mutex_unlock(&data_mutex);
		// usleep(50);
	}
	close(sockfd);
}


void map()
{
	printf(" %c | %c | %c \n",x[0],x[1],x[2]);
	printf("------------\n");
	printf(" %c | %c | %c \n",x[3],x[4],x[5]);
	printf("------------\n");
	printf(" %c | %c | %c \n",x[6],x[7],x[8]);
}

void printmenu(){
    printf("\n======================   Let's play OOXX   =========================\n");
	printf("=                    <<<Choose a function>>                        =\n");
	printf("= menu      --> Show the function menu                             =\n");
	printf("= user      --> Show who is online                                 =\n");
    printf("= @fd       --> send invitation and establish a connection         =\n");
    printf("= #(1~9)    --> Choose place(Not game start or you ture, Blocked)  =\n");                   
	printf("= quit      --> Exit                                               =\n");
	printf("====================================================================\n");
}

int iswin(char le)
{
	if(x[0] == le && x[1] == le && x[2] == le)return 1;
	if(x[3] == le && x[4] == le && x[5] == le)return 1;
	if(x[6] == le && x[7] == le && x[8] == le)return 1;
	if(x[0] == le && x[3] == le && x[6] == le)return 1;
	if(x[1] == le && x[4] == le && x[7] == le)return 1;
	if(x[2] == le && x[5] == le && x[8] == le)return 1;
	if(x[0] == le && x[4] == le && x[8] == le)return 1;
	if(x[2] == le && x[4] == le && x[6] == le)return 1;
	return 0;
}

int istie()
{
	int i;
	for(i=0;i<9;i++)
	{
		if(x[i]==' ')return 0;
	}
	return 1;
}


int main(){
	while(1){
        printf("[INFO] 請輸入帳號 :");
        scanf("%s",name);
        printf("[INFO] 請輸入密碼 :");
        scanf("%s",passwd);
        strcat(name,":");
        strcat(name,passwd);
        strcat(name,"@");
        init();
        while(1){
            int ok = start();
            if(ok == 0) return 0;
            else break;
        }
    }
	return 0;
}