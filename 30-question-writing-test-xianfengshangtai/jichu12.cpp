#include <cstdlib>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <netdb.h>
using namespace std;

#define MAXSIZE 80
#define BACKLOG 10
#define BUFSIZE 100
#define PORT 3490

void str_ser(int);

int main(int argc,char **argv)
{
    int sockfd,ret,con_fd,pid;
    struct sockaddr_in my_addr;
    struct sockaddr_in their_addr;
    int sin_size;   

    sockfd=socket(AF_INET,SOCK_STREAM,0);
    if(sockfd<0)
    {
        cout<<"error in socket create"<<endl;
        exit(1);
    }
    cout<<"sockfd="<<sockfd<<endl;

    my_addr.sin_family=AF_INET;
    my_addr.sin_port=htons(PORT);
    my_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    bzero(&(my_addr.sin_zero),8);
    ret=bind(sockfd,(struct sockaddr*)&my_addr,sizeof(struct sockaddr));

    if(ret<0)
    {
        cout<<"error in binding"<<endl;
        exit(1);
    }
    cout<<"bind ret="<<ret<<endl;
    ret=listen(sockfd,BACKLOG);
    if(ret<0)
    {
        cout<<"error in listening"<<endl;
        exit(1);
    }

    cout<<"listen ret="<<ret<<endl;

    while(true)
    {
        sin_size=sizeof(struct sockaddr_in);
        con_fd=accept(sockfd,(struct sockaddr *)&their_addr,(socklen_t*)&sin_size);
        if(con_fd<0)
        {
            cout<<"error in accept"<<endl;
            exit(1);
        }
        cout<<"accept con_fd="<<con_fd<<endl;
        if((pid=fork())==0)
        {
            close(sockfd);
            str_ser(con_fd);
            close(con_fd); 
            exit(0);
        }else
            close(con_fd);

    }
    close(sockfd);
    exit(0);
}

void str_ser(int sockfd)
{
    char recvs[MAXSIZE];
    int n=0;
    pid_t pid=getpid();
    double f=10.123456;

    for(int i=0;i<100;i++)
    {
        double x=f*i;

        if((n=write(sockfd,&x,sizeof(double)))<=0)
        {
          printf("pid=%d Server write data error!Error code=%d\n",pid,n);
          close(sockfd);
          return;
        }
        printf("pid=%d Server write data %f\n",pid,x);
    }
  
}

