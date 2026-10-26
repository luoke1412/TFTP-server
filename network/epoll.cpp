//这里演示epoll实现并发操作
//epoll的具体数据结构：
/*
epitem(红黑树节点)      每个 epfd 的 fd_tree    正向: 我在监听哪些fd + 要什么事件
file->ep_links(双向链表)  每个被监听的 fd 上      反向: 我被哪些 epfd 监听(事件来了该叫醒谁)
ep->ready_list(双向链表)  每个 epfd 上            就绪队列: 当前就绪的 epitem，wait 取它
*/
#include<iostream>
#include<string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<stdio.h>
#include<sys/epoll.h>

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

/*****************epoll部分************************* */
//1.创建一个epoll树并拿到fd
int epfd=epoll_create(1);
if(epfd==-1)
{
    perror("epoll_create error");
    return -1;
}

printf("epoll_create success,epfd=%d\n",epfd);

//2.将待监听的描述符注册到这棵树中
    //先定义事件信息结构体
epoll_event ev;
ev.data.fd=sfd;
ev.events=EPOLLIN; //读事件

if(epoll_ctl(epfd,EPOLL_CTL_ADD,sfd,&ev)==-1)
{
    perror("epoll_ctl erorr");
    return -1;
}

//3.创建一个容器(内核用的是链表)存储就绪的描述符
epoll_event evs[1024];
int len=sizeof(evs)/sizeof(evs[0]);

struct sockaddr_in sock_arr[1024]; 
while(1)
{
    int num=epoll_wait(epfd,evs,len,-1); 
    //返回值是就绪的fd数量
    if(num==-1)
    {
        perror("epoll_wait error");
        return -1;
    }

    //接下来遍历evs看看是要触发事件了
    for(int i=0;i<num;i++)
    {
        if(evs[i].data.fd==sfd) //是sfd就负责连接
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
                    
            sock_arr[newfd]=cin;

            //并且将新客户注册到树中啊
            epoll_event ev;
            ev.data.fd=newfd;
            ev.events=EPOLLIN; //读事件

            if(epoll_ctl(epfd,EPOLL_CTL_ADD,newfd,&ev)==-1)
            {
                perror("epoll_ctl erorr");
                return -1;
            }
        }

        else  //普通服务器就负责通信
        {
            int newfd=evs[i].data.fd;
            //5.收发数据
            //5.1 定义容器接收数据
            char buf[128];

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
                
                //先从树中删除，再关闭套接字
                epoll_ctl(epfd,EPOLL_CTL_DEL,newfd,NULL);
                close(newfd);
               
                continue; //进入下一次循环
            }
            buf[res] = '\0';
            printf("recv [%s:%d]%s\n",inet_ntoa(sock_arr[newfd].sin_addr),ntohs(sock_arr[newfd].sin_port),buf);

            //发过去
             strcat(buf,"*_*"); //加个笑脸发送过去
            if(send(newfd,buf,strlen(buf),0)==-1)
            {
                perror("send error");
                return -1;
            }
            printf("send success\n");
            //6.关闭套接字(两个都要)
            //close(sfd); //这个不要关哦，还要和其他客户端连接
            //close(newfd);
        }
    }

    
}
    //关闭这两个文件描述符
    close(sfd); 
    close(epfd);
    return 0;
}