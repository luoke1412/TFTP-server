//这里演示查看和设置网络属性之  设置套接字地址快速重用
#include<iostream>
#include<string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<stdio.h>

#define SER_IP  "172.17.165.43"
#define SER_PORT 9000
#include<iostream>
#include<stdio.h>
using namespace std;

int main(int argc, const char *argv[])
{
    //创建套接字
    int sfd;
    sfd=socket(AF_INET,SOCK_STREAM,0);
    if(sfd==-1)
    {
        perror("socket error");
        return -1;
    }
    printf("socket success!\n");
    //绑定地址
    sockaddr_in addr;
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=inet_addr(SER_IP);
    addr.sin_port=SER_PORT;

    if(bind(sfd,(const sockaddr*)&addr,sizeof(addr))==-1)
    {
        perror("bind error");
        return -1;
    }
    printf("bind success!\n");

    //查看一下是否可以重用套接字地址
    int reuse=-1;
    socklen_t len=sizeof(reuse);
    if(getsockopt(sfd,SOL_SOCKET,SO_REUSEADDR,&reuse,&len)==-1) //sol: socket layer
    {
        perror("getsockopt error");
        return -1;
    }

    printf("reuse=%d\n",reuse);  //0:表示默认不可用   1：默认可用

    //设置为1：现在外部手动改为1，再传入进去
    reuse=1;
    if(setsockopt(sfd,SOL_SOCKET,SO_REUSEADDR,&reuse,len)==-1)
    {
        perror("setsockopt erorr");
        return -1;
    }

    //验证,获取默认值
    if(getsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &reuse, &len)==-1)
    {
        perror("getscokopt error");
        return -1;
    }
    printf("reuse = %d\n", reuse);
}