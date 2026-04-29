#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <arpa/inet.h>
#include <string.h>
using namespace std;
#define MAXSIZE 80

#define IP "10.0.2.15"
#define PORT 3490

void str_cli(FILE *,int);

int main(int argc,char **argv)
{
    int sockfd,ret,len;
    struct sockaddr_in ser_addr;
     
     sockfd=socket(AF_INET,SOCK_STREAM,0);
     if(sockfd<0) {
         cout<<"error in socket"<<endl;
         exit(1);
     }
    ser_addr.sin_family=AF_INET;
    ser_addr.sin_port=htons(PORT);   
    inet_aton(IP,&ser_addr.sin_addr);  //转换成网络字节序;
    bzero(&(ser_addr.sin_zero),8);
    cout<<"server ip="<<inet_ntoa(ser_addr.sin_addr)<<endl; //转换成主机字节序;
    ret=connect(sockfd,(const sockaddr *)&ser_addr,sizeof(struct sockaddr));
    cout<<"connect ret="<<ret<<endl; 
    //cout<<"errno="<<errno<<endl; 
    if(ret==-1) 
    {
        cout<<"error connecting"<<endl;
        close(sockfd);
        exit(1); 
    } 
     
    str_cli(stdin,sockfd);
    close(sockfd);
    exit(0);
}

void str_cli(FILE *fp,int sockfd)
{
    char sends[MAXSIZE];
    int n=0; 
    double data=0; 
    for(int i=0;i<100;i++) {        
       if((n=read(sockfd,&data,sizeof(double)))<=0){
           printf("read data error,error code=%d\n",n);
           close(sockfd);
           return;     
           }
       printf("client received data=%f\n",data);
    }

      
}