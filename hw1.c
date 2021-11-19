#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/wait.h>

char webpage[] =
"HTTP/1.1 200 OK\r\n"
"Content-Type: text/html; charset=UTF-8\r\n\r\n"
"<!DOCTYPE html>\r\n"
"<html><head><title>hw1</title>\r\n"
"<style>body { background-color: #00FFFF}</style></head>\r\n"
"<center><h1>405410120</h1><br>\r\n"
"<body><center>"

"<form method='Post' enctype='multipart/form-data'>"
"<center><input type='file' name='yourfile'\r\n>"
"<center><Input Type='Submit' Value='submit'><Input Type='Reset' Value='clear'></form>\r\n\n"

"<h1><h1/>"

"<img src='https://i.ibb.co/ThFd5Pj/article-5bd182cf13ebb.jpg' alt='bf32199b457a11764b1a72b18259ff61' border='0'></a>"

"<br>\r\n";



int main(int argc, char *argv[])
{
	struct sockaddr_in client_addr;
	socklen_t sin_len;
	int fd_server , fd_client;
	char buf[2048];
	int pid;
	

	fd_server = socket(AF_INET, SOCK_STREAM, 0);
	if(fd_server < 0)
	{
		perror("socket");
		exit(1);
	}

	client_addr.sin_family = AF_INET; //地址類型
	client_addr.sin_addr.s_addr = INADDR_ANY; //32位IP(INADDR_ANY為本地) 
	client_addr.sin_port = htons(8080); //16位端口

	//綁定ip和端口 錯誤傳回-1
	if(bind(fd_server, (struct sockaddr *) &client_addr, sizeof(client_addr)) == -1)
	{
		perror("bind");
		close(fd_server);
		exit(1);
	}

	//用來通知OS/network socketfd的socket已經可以接受建立連線 成功傳回0，錯誤傳回-1
	if(listen(fd_server, 10) == -1)
	{
		perror("listen");
		close(fd_server);
		exit(1);
	}

	while(1)
	{
		//成功则返回新的socket 处理代码, 失败返回-1
		fd_client = accept(fd_server, (struct sockaddr *) &client_addr, &sin_len);

		if(fd_client == -1)
		{
			perror("Connection failed....\n");
			continue;
		}

		printf("Got client connection.......\n");
		pid = fork();
		if(!pid)
		{
			/*child process */
			close(fd_server);
			memset(buf, 0, 2048);
			read(fd_client, buf, 2047);

			printf("%s\n", buf);

			write(fd_client, webpage, sizeof(webpage) -1);
			close (fd_client);
			printf("closing...\n");
			exit(0);
		}
		/*parent process*/
		if(pid)
        {
			wait(NULL);
			close(fd_client);
		}
	}
	return 0;
}
