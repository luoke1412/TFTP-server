//实现一个服务器和多个客户端通信的第一个思路--循环accept服务器
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
    //1.创建套接字
    
    int sfd=socket(AF_INET,SOCK_STREAM,0);
    if(sfd==-1)
    {
        perror("socket error");
        return -1;
    }
    printf("socket success!,sfd=%d\n",sfd);

    //2.绑定ip，端口号和地址族
        //2.1 填充要绑定的ip地址,端口号,地址族结构体 。  _in是ipv4专属
    sockaddr_in sin;
    sin.sin_family=AF_INET;
    sin.sin_addr.s_addr=inet_addr(SER_IP); //字符串变网络地址(4字节整数)
    sin.sin_port=htons(SER_PORT);

    if(bind(sfd,(const sockaddr*)&sin,sizeof(sin))==-1)
    {
        perror("bind error");
        return -1;
    }
    printf("bind success!\n");

    //3.设置为监听状态，并开辟连接队列
    if(listen(sfd,128)==-1)
    {
        perror("listen error");
        return -1;
    }
    printf("listen success!\n");

/****************************************** */
//重点在这里，我要让一个服务器连接多个客户端，就得把accept放入到循环中
while(1)
{
    //4.阻塞等待想要建立连接的客户端，并返回一个新的套接字用于实现和客户端通信
        //补：定义一个用于接收客户端套接字信息的结构体
    sockaddr_in cin;
    
    //int newfd=accept(sfd,(sockaddr*)&cin,(socklen_t*)sizeof(cin));
                                            
    //int newfd=accept(sfd,(sockaddr*)&cin,&(socklen_t)sizeof(cin));
    // 这两个都犯了一个同样的错误，sizeof是常量， 要赋值给一个变量才能找到地址
    
    socklen_t socklen=sizeof(cin);
    int newfd=accept(sfd,(sockaddr*)&cin,&socklen);
    if(newfd==-1)
    {
        perror("accept error");
        return -1;
    }                                   //突然想起来，ntoa的形参就是结构体
    printf("socket:[%s:%d] accept success!\n",
        inet_ntoa(cin.sin_addr),ntohs(cin.sin_port));
    //5.收发数据
        //5.1 定义容器接收数据
    char buf[128];
    //循环接收和发送
    while(1)
    {
        bzero(buf,sizeof(buf));
        int res=recv(newfd,buf,sizeof(buf),0); //以阻塞的形式接收数据
        if(res==-1)
        {
            perror("recv error");
            return -1;
        }
        else if(res==0)
        {
            printf("对端已下机\n");
            break;
        }
        buf[res] = '\0';
        printf("recv [%s:%d]%s\n",inet_ntoa(cin.sin_addr),ntohs(cin.sin_port),buf);

        //发过去
        printf("please print new data:");
        fgets(buf,sizeof(buf),stdin);
        buf[strlen(buf)-1]=0; //把换行符覆盖掉

        if(send(newfd,buf,strlen(buf),0)==-1) //0:阻塞等待(别人容器满了)
        {
            perror("send error");
            return -1;
        }
        printf("send success\n");

    }
    //6.关闭套接字(两个都要)
    //close(sfd); //这个不要关哦，还要和其他客户端连接
    close(newfd);
}
    close(sfd); 

    return 0;
}