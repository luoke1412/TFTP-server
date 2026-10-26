//普通服务器
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
using namespace std;
int main(int argc, const char *argv[])
{
    //1.创建套接字(信息的载体)并返回它的文件描述符
    int sfd=-1;
    if((sfd=socket(AF_INET,SOCK_STREAM,0))==-1)  
    //地址族因特网(ipv4), 流式(tcp), 默认协议
    {
        perror("socket error");
        return -1;
    }

    printf("socket create success!sfd=%d\n",sfd);

    //2.绑定-将IP地址和端口号与套接字绑定，以使别人能找到我

        //2.1 填充要绑定的ip地址,端口号,地址族结构体 。  _in是ipv4专属
    struct sockaddr_in sin;
    sin.sin_family=AF_INET;
    sin.sin_addr.s_addr=inet_addr(SER_IP);  //wtm结构体里面是结构体来着

    sin.sin_port=htons(SER_PORT); //传输16位整数嘛

        //2.2填嘛，细节太多了
    if(bind(sfd,(const sockaddr*)&sin,sizeof(sin))==-1)
    {
        perror("bind error");
        return -1;
    }
     cout<<"绑定成功\n";

    //3.套接字启动监听模式(监听)
    if(listen(sfd,128)==-1)
    // 128:已连接队列的最大长度
    //也就是说，最多允许有 128 个已经连好的客户端在后台排队等 accept()
    {
        perror("listen error");
        return -1;
    }
    printf("listen success!\n");

    //4.阻塞等待客户端的连接请求，如果已连接队列中有客户端，则从连接队列中拿取第一个，并创
//建一个用于通信的套接字
        //4.1定义变量，用于接收客户端地址信息结构体
    struct sockaddr_in cin;
    socklen_t socklen=sizeof(cin);
    
    int newfd=accept(sfd,(sockaddr*)&cin,&socklen);
    //int newfd=accept(sfd,NULL,NULL);
    //参数1：服务器套接字文件描述符
    //参数2：用于接收 《客户端地址信息》  结构体 的容器，如果不接收，也可以填NULL
    //参数3：接收参数2的大小，如果参数2为NULL，则参数3也是NULL
    if(newfd==-1)
    {
        perror("accept error");
        return -1;
    }

    printf("[%s:%d]连接成功!\n",inet_ntoa(cin.sin_addr),ntohs(cin.sin_port));
    //5.收发数据
        //5.1定义容器接收数据
    char buf[128]="";

    while(1)
    {
        bzero(buf,sizeof(buf)); //每次先清空容器内容

        //int res=recv(newfd,buf,strlen(buf),0); //0:表示阻塞接收,返回值：接收的个数
          int res=recv(newfd,buf,sizeof(buf),0);  //第三个参数表示可接受的大小
        if(res==0)
        {
            printf("对端已下线\n");
            break;
        }

        printf("服务器接收数据[%s:%d]:%s\n",inet_ntoa(cin.sin_addr),ntohs(cin.sin_port)
        ,buf);

        //-------
        strcat(buf,"*_*"); //加个笑脸发送过去
        if(send(newfd,buf,strlen(buf),0)==-1)
        {
            perror("send error");
            return -1;
        }
        printf("服务器端已发送:%s\n",buf);
    }
    

    //6.关闭套接字
    close(sfd);

    close(newfd);
    return 0;
}
