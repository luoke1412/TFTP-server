//演示UDP服务器端
/*
ps:一定要理解 原理，原理！  UDP是不支持连接的，无法从自己的套接字直接到对方的套接字
    如果你要发数据，要指明对方的套接字
    如果你要接收数据，也可以定义容器按需接收对方的套接字信息  至于从自己的套接字里面取数据？
    明显嘛，自己的套接字到容器  以及容器到套接字 是自动执行的
*/
#include<iostream>
#include<string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<stdio.h>
#include<sys/select.h>
#define SER_IP "172.17.165.43"
#define SER_PORT 9005
using namespace std;
int main(int argc, const char *argv[])
{
    //1.创建套接字
    int sfd=socket(AF_INET,SOCK_DGRAM,0);
    if(sfd==-1)
    {
        perror("socket error");
        return -1;
    }
    printf("socket success!\n");
    //2.绑定地址(必须选) 为了让客户找到服务器
    sockaddr_in sin;
    sin.sin_family=AF_INET;
    sin.sin_addr.s_addr=inet_addr(SER_IP);
    sin.sin_port=htons(SER_PORT);
    socklen_t len=sizeof(sin);

    if(bind(sfd,(const sockaddr*)&sin,len)==-1)
    {
        perror("bind error"); 
        return -1;
    }
    printf("bind success!\n");
    //3.收发数据(因为UDP是无连接的，所以在发送的时候要传套接字地址)
    while (1)
    {
        //定义容器接收从客户端发来到服务器套接字的数据(X)
        char buf[128]="";
        bzero(buf,sizeof(buf));
        //错啦，两部分参数，一个是接收数据，一个是接收发送方的套接字信息的容器
            //定义容器
        sockaddr_in cin;
        len=sizeof(cin);
        int ret=recvfrom(sfd,buf,sizeof(buf),0,(sockaddr*)&cin,&len);
        if(ret==-1)
        {
            perror("recvfrom error");
            return -1;
        }
        else if(ret==0)
        {
            //printf("对端已下线\n");  没连接呀
            continue;
        }
        buf[ret]=0;
        printf("receive success!buf=%s\n",buf);

        //发送数据，前提是你接受了对方的套接字地址
        strcat(buf,"*_*");
        //发送数据，是要先发送到
        if(sendto(sfd,buf,strlen(buf),0,(const sockaddr*)&cin,len)==-1)
        {
            perror("send error");
            return -1;
        }
        printf("send success!\n");
    }
    
    return 0;
}