//为了让两个阻塞函数accept 和 recv并发执行，这里演示多线程
//具体思路就是主线程负责 连接和 创建子线程，有多少连接就创建多少   最后还要记得回收资源
//子线程负责和客户端通信

#include<iostream>
#include<string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include<pthread.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<stdio.h>

#define SER_IP  "172.17.165.43"
#define SER_PORT 9000
#include<iostream>
#include<stdio.h>
using namespace std;


//int newfd;  //newfd当成全局变量(X) 所有线程是共享的啊！

struct Info
{
    int newfd;  //用于通信的套接字
    sockaddr_in cin; //客户端套接字地址
};


void* task(void* arg)  //( *)arg= &info
{
    struct Info* info=(struct Info*)arg;
    int newfd=info->newfd;
    sockaddr_in cin=info->cin;

    //5.收发数据
        //5.1 定义容器接收数据
    char buf[128];
    //循环接收和发送
    while(1)
    {
        
        int res=recv(newfd,buf,sizeof(buf),0); //以阻塞的形式接收数据
        if(res==-1)
        {
            perror("recv error");
            return NULL;
        }
        else if(res==0)
        {
            printf("对端已下机\n");
            break;
        }
        buf[res] = '\0';
        printf("recv [%s:%d]%s\n",inet_ntoa(cin.sin_addr),ntohs(cin.sin_port),buf);

        //发过去
        strcat(buf,"已接收\n");
        if(send(newfd,buf,strlen(buf),0)==-1) //0:阻塞等待(别人容器满了)
        {
            perror("send error");
            return NULL;
        }
        printf("send success\n");

    }
    //6.关闭套接字(两个都要)
    //close(sfd); //这个不要关哦，还要和其他客户端连接
    close(newfd);
    pthread_exit(NULL);
}


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
    struct Info info;
    //int newfd=accept(sfd,(sockaddr*)&cin,(socklen_t*)sizeof(cin));
                                            
    //int newfd=accept(sfd,(sockaddr*)&cin,&(socklen_t)sizeof(cin));
    // 这两个都犯了一个同样的错误，sizeof是常量， 要赋值给一个变量才能找到地址
    
    socklen_t socklen=sizeof(cin);
    info.newfd=accept(sfd,(sockaddr*)&cin,&socklen);
    if(info.newfd==-1)
    {
        perror("accept error");
        return -1;
    }                                   //突然想起来，ntoa的形参就是结构体
    printf("socket:[%s:%d] accept success!\n",
        inet_ntoa(info.cin.sin_addr),ntohs(info.cin.sin_port));

    info.cin=cin;
    /**********************************************/
    pthread_t tid=-1; //线程号
    if(pthread_create(&tid,NULL,task,&info)!=0)
    {
        printf("pthread_create error!\n");
        return -1;
    }

    //主线程该干嘛？回收资源喽
    //pthread_join(tid,NULL);  不能阻塞回收

    //如果不管，子线程变成僵尸态，等着主线程给它收尸
    //设置成分离态后，不归主线程管，子线程死后自动被OS回收。 也就是其生命由客户端决定
    pthread_detach(tid);  
}
    close(sfd); 

    return 0;
}