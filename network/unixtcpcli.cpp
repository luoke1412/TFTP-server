//流式域套接字客户端
#include<iostream>
#include<string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<stdio.h>
#include<sys/un.h>
// 绑定（bind）服务端：IP 必须是本机实际拥有的地址。可以通过 ip addr或 ifconfig查看。
// 如果想让程序在所有网卡上监听，应使用通配符 0.0.0.0
// （代码中对应 INADDR_ANY）；如果只限本机访问，用回环地址 127.0.0.1
using namespace std;
int main(int argc, const char *argv[])
{
 //1.创建套接字
    int cfd=socket(AF_UNIX,SOCK_STREAM,0);
    if(cfd==-1)
    {
        perror("socket error");
        return -1;
    }
    printf("socket success,cfd=%d\n",cfd);


/**************************************** */
    //1.5绑定时用到的套接字文件，必须是不存在的，所以要判断存不存在，如果存在
    //先删掉
    if(access("./cli",F_OK)==0) 
    {
        //说明文件已经存在，需要删除
        if(unlink("./cli")==-1)
        {
            perror("unlink error");
            return -1;
        }
    }


 //2.绑定套接字文件(必选，系统不会随机创建套接字文件)
    sockaddr_un cun;
    cun.sun_family=AF_UNIX;
    strcpy(cun.sun_path,"./cli");

    
    if(bind(cfd,(const sockaddr*)&cun,sizeof(cun))==-1)
    {
        perror("bind error");
        return -1;
    }
    printf("bind success\n");
    
 //3.连接服务器端套接字
        //3.1：填充要连接的结构体信息
    sockaddr_un sun;
    sun.sun_family=AF_UNIX;
    strcpy(sun.sun_path,"./unix");

        //3.2 连接
    if(connect(cfd,(const sockaddr*)&sun,sizeof(sun))==-1)
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
        printf("客户端接收数据：[%s]:%s\n",sun.sun_path,buf);
    }
    

 //5.关闭套接字
    close(cfd);
    return 0;
}