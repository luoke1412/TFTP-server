/*
这里演示域套接字实现同一主机不同进程之间的通信，类似IPC
因为不需要跨网络了，所以不需要IP和端口
那么它存储信息的载体是啥呢 套接字类型文件 s
bind绑定的是套接字文件，而不再是IP和端口了
*/

#include<iostream>
#include<string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<stdio.h>
#include<sys/select.h>
#include<sys/un.h>
#include<fcntl.h>
using namespace std;
int main(int argc, const char *argv[])
{
    //1.创建套接字(信息的载体)并返回它的文件描述符
    int sfd=-1;
    if((sfd=socket(AF_UNIX,SOCK_STREAM,0))==-1)  
    //地址族因特网(ipv4), 流式(tcp), 默认协议
    {
        perror("socket error");
        return -1;
    }

    printf("socket create success!sfd=%d\n",sfd);


/**************************************** */
    //1.5绑定时用到的套接字文件，必须是不存在的，所以要判断存不存在，如果存在
    //先删掉
    if(access("./unix",F_OK)==0) 
    {
        //说明文件已经存在，需要删除
        if(unlink("./unix")==-1)
        {
            perror("unlink error");
            return -1;
        }
    }

    //2.绑定套接字和套接字类型文件(传路径)

        //2.1 填充要绑定的ip地址,端口号,地址族结构体 。  _in是ipv4专属
    struct sockaddr_un sun;
    sun.sun_family=AF_UNIX;
    //sun.sun_path="unix";  不能等号赋值，那是初始化
    strcpy(sun.sun_path,"./unix");
    socklen_t len=sizeof(sun);
    //sin.sin_port=htons(SER_PORT); //传输16位整数嘛

        //2.2填嘛，细节太多了
    if(bind(sfd,(const sockaddr*)&sun,len)==-1)
    {
        perror("bind error");
        return -1;
    }
    std::cout<<"绑定成功\n";

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
    struct sockaddr_un cun;
    socklen_t socklen=sizeof(cun);
    
    int newfd=accept(sfd,(sockaddr*)&cun,&socklen);
    //int newfd=accept(sfd,NULL,NULL);
    //参数1：服务器套接字文件描述符
    //参数2：用于接收 《客户端地址信息》  结构体 的容器，如果不接收，也可以填NULL
    //参数3：接收参数2的大小，如果参数2为NULL，则参数3也是NULL
    if(newfd==-1)
    {
        perror("accept error");
        return -1;
    }

    printf("[%s]连接成功!\n",cun.sun_path);
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

        printf("服务器接收数据[%s]:%s\n",cun.sun_path,buf);

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
