/*
 *  client.c
 *  
 *
 *  Created by upendra singh bartwal.
 *  Copyright 2015 IIT. All rights reserved.
 *
 */
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 
#include<dirent.h>
#include<sys/sendfile.h>
#include<sys/stat.h>
#include<fcntl.h>
#include <termios.h>
char s[100],p[100];
int sockfd = 0;
pthread_t threads1, threads2;
struct dirent **fileList;
int  Fileno;
extern  int alphasort();
int kbhit(void)
{
  struct termios oldt, newt;
  int ch;
  int oldf;
 
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
 
  ch = getchar();
 
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);
 
  if(ch != EOF)
  {
    ungetc(ch, stdin);
    return 1;
  }
 
  return 0;
}
void ls()
{
	
	int i;
 	char path[100] = {"."};
	Fileno=scandir(path,&fileList,NULL,alphasort);
	
}
void pwd()
{
	char pathp[100];
	getcwd(pathp,100);
	printf("%s \n",pathp);
}

void *writes(void *arg)
{
	int flag,i,readfile,size,sizet;
	long long sizetemp=0;
	char command[100],filen[100],ch;
	struct stat stat_buffer;
	off_t offset=0;
	while(1)
	{
		 
		printf("myftp>");		
		gets(p);
		sscanf(p,"%s %s",command,filen);
		flag=0;
		write(sockfd,p,100);
		if(p[0]!='!')
		{
			
			if(!strcmp(command,"put"))
			{
				
				ls();
				for(i=0;i<Fileno;i++)
				{
					if(!strcmp(fileList[i]->d_name,filen))
					{
						flag=1;
						break;
					}
				}
				if(flag==0)
				{
					printf("No such file exist\n");
				}
				else
				{
					sizetemp=0;
					readfile=open(filen,O_RDONLY);
					fstat(readfile,&stat_buffer);
					write(sockfd,&stat_buffer.st_size,sizeof(int));
					while(1)
						{
														
							if(kbhit())
							{
								ch=getchar();
								if(ch=='P'||ch=='p')
								{
									while(1)
									{
									if(kbhit())
									{
									ch=getchar();
									if(ch=='U'||ch=='u')							
									break;
									}
						
									}				
								}
							
							}
							printf("\e[1;1H\e[2J");
							size=read(readfile,p,100);
							//printf("%d\n",size);
							if(size==0)
							break;
							else
								{
									write(sockfd,p,size);			
								}
							p[0]=0;
							sizetemp+=size;	
							printf("Uploaded::%f\n",100*((float)sizetemp/(float)stat_buffer.st_size));
							
							
						}	
						printf("Upload complete\n");
				}
			}
			else if(!strcmp(command,"get"))
			{
				read(sockfd,&size,sizeof(int));
				if(size<0)
				{
					printf("File does not exist");					
				}
				else
				{
					readfile=open(filen,O_WRONLY|O_CREAT,777);
					sizetemp=0;
					while(1)
					{	
					if(kbhit())
					{
						ch=getchar();
						if(ch=='p'||ch=='P')
						{
							write(sockfd,"p",2);
							while(1)
									{
									if(kbhit())
									{
									ch=getchar();
									if(ch=='U'||ch=='u')							
									break;
									}
							}
							write(sockfd,"p",2);
						}
						else
						{
							write(sockfd,"h",2);
						}

					}
					else
						{
							write(sockfd,"h",2);
									
						}
					printf("\e[1;1H\e[2J");	
					sizet=read(sockfd,p,100);
					if(sizet==0)
					break;	
					write(readfile,p,sizet);
					//printf("%d\n",sizet);
					//printf("\nsize remaining::%d\n",(size-sizet)/1024);
					sizetemp+=sizet;
					
					printf("Downloaded::%f\n",100*((float)sizetemp/(float)size));	
										
					if(100*((float)sizetemp/(float)size)==100)
					break;					
					p[0]=0;
					}
					printf("Dwonlaod complete\n");
				}
				
			}
			else if(!strcmp(command,"ls"))
			{
				read(sockfd,&sizet,sizeof(int));
				printf("%d\n",sizet);
				for(i=0;i<sizet;i++)
				{
					read(sockfd,s,100);
					printf("%s\n",s);
					s[0]=0;
				}
			}
			
		}
		else
		{
			if(!strcmp(command,"!ls"))
			{
				ls();
				for(i=0;i<Fileno;i++)
				{
				printf("%s\n",fileList[i]->d_name);
				}
	
			}
			else if(!strcmp(command,"!pwd"))
			{
				pwd();	
			}
			
		}	
	}
	return(NULL);
}
int main(int argc, char *argv[])
{
    int  n = 0;
    char recvBuff[1024];int temp;
    struct sockaddr_in serv_addr; 
    pthread_attr_t pthread_custom_attr1, pthread_custom_attr2;
    pthread_attr_init(&pthread_custom_attr1);
    pthread_attr_init(&pthread_custom_attr2);
 
    if(argc != 2)
    {
        printf("\n Usage: %s <ip of server> \n",argv[0]);
        return 1;
    } 
    memset(recvBuff, '0',sizeof(recvBuff));
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    } 
    memset(&serv_addr, '0', sizeof(serv_addr)); 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(5000); 
    printf("Server address used is: %s\n", argv[1]);
    if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0)
    {
        printf("\n inet_pton error occured\n");
        return 1;
    } 
	
    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
		printf("\n Error : Connect Failed \n");
		return 1;
    }
	pthread_create(&threads2, &pthread_custom_attr2, &writes, (void *)(p));	
	pthread_join(threads1,NULL);
	pthread_join(threads2,NULL);        	
   return 0;
}
