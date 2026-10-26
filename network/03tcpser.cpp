/*
循环服务器确实能实现一个服务器连接多个客户端，但是因为有两个阻塞函数 accept--recv
服务器卡在recv了，必须要一个客户端下线另一个客户端才能连接
于是我们想到有没有办法实现accept，recv同时执行呢？ 引入并发机制

先试试多进程
---具体思路：主进程专门负责待客(阻塞连接请求的客户端) 
            每连接一个客户端就fork一个子进程，
            子进程专门负责通过newfd通信。      ok了~
*/
#include<iostream>
#include<string.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<stdio.h>

#define SER_IP  "172.17.165.43"
#define SER_PORT 9000
#include<iostream>
#include<stdio.h>
using namespace std;

//自定义信号处理函数
void handler(int signo)
{
if(signo == SIGCHLD)
{
while(waitpid(-1, NULL, WNOHANG) > 0); //循环以非阻塞的形式回收僵尸进程
}
}

int main(int argc, const char *argv[])
{

    //将子进程发送的SIGCHLD信号连接到自定义信号处理函数中
    if(signal(SIGCHLD, handler) == SIG_ERR)
    {
        perror("signal error");
        return -1;
    }


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
    
    socklen_t socklen=sizeof(cin);
    int newfd=accept(sfd,(sockaddr*)&cin,&socklen);
    if(newfd==-1)
    {
        perror("accept error");
        return -1;
    }                                   //突然想起来，ntoa的形参就是结构体
    printf("socket:[%s:%d] accept success!\n",
        inet_ntoa(cin.sin_addr),ntohs(cin.sin_port));


    /****************************/
    //7.克隆子进程
    pid_t pid=fork();
    
    if(pid>0) 
    {
        //父进程---不管recv，也不要阻塞，他要上去accept

        //问题来了，如何回收进程
        //wait(NULL);  不行，这是阻塞函数
        
        //waitpid(-1, NULL, WNOHANG); //以非阻塞形式回收？

        //-》不，咱们用信号+非阻塞回收

        //父进程关闭newfd
        close(newfd);
    }

    else if(pid==0)
    {
        //子进程

        //先关闭sfd，因为子进程用不上
        close(sfd);
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
                exit(1);  //只退出本进程，不影响其他在工作的进程
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
               exit(1);
            }
            printf("send success\n");

        }
        //6.关闭套接字(两个都要)
        //close(sfd); //这个不要关哦，还要和其他客户端连接
        close(newfd);

        exit(0);    //进程退出
    }
    else 
    {
        perror("fork errror");
        exit(1);
    }
    
}
    close(sfd); 

    return 0;
}