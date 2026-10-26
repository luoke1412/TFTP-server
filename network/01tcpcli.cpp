//普通客户端
//写之前都先理一下TCP传输数据的思路
//这个要记住： sfd负责监听(listen)和同意连接和创建负责通信的套接字(accept)
// newfd才是负责和客户端直接通信的
#include<iostream>
#include<string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<stdio.h>
// 绑定（bind）服务端：IP 必须是本机实际拥有的地址。可以通过 ip addr或 ifconfig查看。
// 如果想让程序在所有网卡上监听，应使用通配符 0.0.0.0
// （代码中对应 INADDR_ANY）；如果只限本机访问，用回环地址 127.0.0.1
#define SER_IP  "172.17.165.43"
#define SER_PORT 9000
#define CLI_IP " 192.168.248.153"
#define CLI_PORT 8000
using namespace std;
int main(int argc, const char *argv[])
{
 //1.创建套接字
    int cfd=socket(AF_INET,SOCK_STREAM,0);
    if(cfd==-1)
    {
        perror("socket error");
        return -1;
    }
    printf("socket success,cfd=%d\n",cfd);
 //2.绑定IP地址和端口号(可选)  
            //(斯，如果服务器想知道客户端的ip和端口可用。如果只是发数据，本来就有newfd了)
    sockaddr_in cin;
    cin.sin_family=AF_INET;
    cin.sin_addr.s_addr=inet_addr(CLI_IP);
    cin.sin_port=htons(CLI_PORT);
    
    if(bind(cfd,(const sockaddr*)&cin,sizeof(cin))==-1)
    {
        perror("bind error");
        return -1;
    }
    printf("bind success\n");
    
 //3.连接服务器端套接字
        //3.1：填充要连接的结构体信息
    sockaddr_in sin;
    sin.sin_family=AF_INET;
    sin.sin_addr.s_addr=inet_addr(SER_IP);  //wtm结构体里面是结构体来着
    sin.sin_port=htons(SER_PORT); //传输16位整数嘛

        //3.2 连接
    if(connect(cfd,(const sockaddr*)&sin,sizeof(sin))==-1)
    {
        perror("connect error");
        return -1;
    }
    printf("connect success!\n");

 //4.收发数据
    
    char buf[128];
    while(1)
    {
        bzero(buf,sizeof(buf));
        //发送数据
        fgets(buf,sizeof(buf),stdin);
        buf[strlen(buf)-1]=0;
        //前面cfd已经和服务器套接字建立连接了，于是通过cfd将数据发送过去
        send(cfd,buf,strlen(buf),0);
        printf("客户端已发送:%s\n",buf);
        //接收数据
        if(recv(cfd,buf,sizeof(buf),0)==0)
        {
            printf("对端已下线\n");
            break;
        } 
        
        //buf[strlen(buf)-1]=0; 
        printf("客户端接收数据：[%s,%d]:%s\n",inet_ntoa(sin.sin_addr),
        ntohs(sin.sin_port),buf);
    }
    

 //5.关闭套接字
    close(cfd);
    return 0;
}