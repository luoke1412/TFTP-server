//来看一下客户端  收发数据那里，fgets(从终端读数据)以及recv(从服务器端读数据) 是两个阻塞函数。
//如果说啊，服务器突然想给客户端发送数据，而客户端还在fgets阻塞着，那怎么行。  (c-s是相对的)
//也就是说这里两个阻塞函数同样也要并发执行。
//select可以，但是这里演示poll

#include<iostream>
#include<string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<poll.h>
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

/******************************************************************* */
//将这两个文件描述符放入poll数组中  分别是0号，cfd
struct pollfd fds[2];
fds[0].fd=0;
fds[0].events=POLLIN; //读事件

fds[1].fd=cfd;
fds[1].events=POLLIN;




 //4.收发数据
    
    char buf[128];
    while(1)
    {

        int res=poll(fds,2,-1);
        if(res==-1)
        {
            perror("poll error");
            return -1;
        }
        else if(res==0)
        {
            printf("time out!\n");
            continue;
        }

        //至此，至少有一个触发了事件的描述符

        if(fds[0].events&fds[0].revents)
        {
             bzero(buf,sizeof(buf));
            //发送数据
            fgets(buf,sizeof(buf),stdin);
            buf[strlen(buf)-1]=0;
            //前面cfd已经和服务器套接字建立连接了，于是通过cfd将数据发送过去
            send(cfd,buf,strlen(buf),0);
            printf("客户端已发送:%s\n",buf);
        }
       
        if(fds[1].events&fds[1].revents)
        {
            bzero(buf,sizeof(buf));
            //接收数据
            int len;
            if((len=recv(cfd,buf,sizeof(buf),0))==0)
            {
                printf("对端已下线\n");
                break;
            } 
            
            buf[len]=0; 
            printf("客户端接收数据：[%s,%d]:%s\n",inet_ntoa(sin.sin_addr),
            ntohs(sin.sin_port),buf);
        }
        
    }
    

 //5.关闭套接字
    close(cfd);
    return 0;
}