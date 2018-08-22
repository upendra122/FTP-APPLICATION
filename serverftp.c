/*
By upendra singh bartwal
*/
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h> 
#include<dirent.h>
#include<sys/stat.h>
#include<fcntl.h>

char pathp[100];
void pwd()
{
	
	getcwd(pathp,100);
}
struct dirent **fileList;
int  Fileno;
extern  int alphasort();
void ls()
{
	
	int i;
 	char path[10] = {"."};
	Fileno=scandir(path,&fileList,NULL,alphasort);
	
}
char s[100],p[100];
char buff[100];
int *connfd;    
pthread_t threads1[100], threads2[100];
void *reads(void *arg)
{		
		int i,sizet;
		long long sizetemp=0;
		char command[100],filen[100];
		struct stat stat_buffer;
		int *t=(int *)arg,size;
		int fileread;	
		ssize_t len;
		while(1)
			{
			read(*t,s,100);
			sscanf(s,"%s %s",command,filen);
			if(!strcmp(command,"ls"))
			{
				ls();
				write(connfd[0],&Fileno,sizeof(int));
				for(i=0;i<Fileno;i++)
					{
						strcpy(s,fileList[i]->d_name);
						write(connfd[0],s,100);	
					}	 	 
			}
			else if(!strcmp(command,"pwd"))
			{
				pwd();
				write(connfd[0],pathp,100);
			}
			else if(!strcmp(command,"put"))
			{
				read(connfd[0],&size,sizeof(int));		
				fileread=open(filen,O_WRONLY|O_CREAT,777);
				while(1)
				{				
					sizet=read(connfd[0],buff,100);
					if(sizet==0)
					break;	
					write(fileread,buff,sizet);
					//printf("size remaining::%d\n",(size-sizet)/1024);
					//size=(size-sizet)/1024;
					buff[0]=0;
				}
			}
			else if(!strcmp(command,"get"))
			{		
				fileread=open(filen,O_RDONLY);
				fstat(fileread,&stat_buffer);
				/*if(fileread<0)
				{
					sizet=-1;
					write(connfd[0],&sizet,sizeof(int));
				}*/
				//else
				{
				write(connfd[0],&stat_buffer.st_size,sizeof(int));
				while(1)
				{		
					read(connfd[0],buff,100);
					if(buff[0]=='p'||buff[0]=='P')
					{
						read(connfd[0],buff,100);
					}
					buff[0]=0;		
					sizet=read(fileread,buff,100);
					if(sizet==0)
					break;	
					write(connfd[0],buff,sizet);
					buff[0]=0;
				}
				}
			}
			}
		return (NULL);	
}
int main(int argc, char *argv[])
{
    int listenfd = 0;int i;	
    int *p;
    connfd=malloc(sizeof(int)*10);	
    for(i=0;i<2;i++)
    connfd[i]=0;
    struct sockaddr_in serv_addr; 
    pthread_attr_t pthread_custom_attr1, pthread_custom_attr2;
    char sendBuff[1025];
    int temp;
    time_t ticks; 
    pthread_attr_init(&pthread_custom_attr1);
    pthread_attr_init(&pthread_custom_attr2);
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff)); 	
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5000); 
    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 
    listen(listenfd, 10); 
    while(1)
    {	
	for(i=0;i<10;i++)
	{
	
 	   	connfd[i] = accept(listenfd,(struct sockaddr*)NULL, NULL);
 	  	printf("I am reading from client\n");
 	   	printf("client requested\n");	
		pthread_create(&threads1[i], &pthread_custom_attr1,&reads, &connfd[i]);		
	}
	for(i=0;i<10;i++)
	{	
		pthread_join(threads1[i],NULL);
		       	
	}
	close(connfd[1]);
        sleep(1);
   }
}
