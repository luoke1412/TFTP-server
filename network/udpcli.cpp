//演示UDP服务器端
#include<iostream>
#include<string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<stdio.h>
#include<sys/select.h>
#define SER_IP "172.17.165.43"
#define SER_PORT 9005   
#define CLI_IP "172.30.16.1"
#define CLI_PORT 9005  
using namespace std;
int main(int argc, const char *argv[])
{
    //1.创建套接字
    int cfd=socket(AF_INET,SOCK_DGRAM,0);
    if(cfd==-1)
    {
        perror("socket error");
        return -1;
    }
    printf("socket success!\n");
    //2.绑定地址(可选，如果不选也是随机分配)

    //printf("bind success!\n");
    //3.收发数据(因为UDP是无连接的，所以在发送的时候要传套接字地址)


    sockaddr_in sin;
    sin.sin_family=AF_INET;
    sin.sin_addr.s_addr=inet_addr(SER_IP);
    sin.sin_port=htons(SER_PORT);
    socklen_t len=sizeof(sin);

    while (1)
    {
        //定义容器发送数据
        char buf[128]="";
       // bzero(buf,sizeof(buf));

        int res=read(0,buf,sizeof(buf));
        buf[res-1]=0; //结束符，同时吞掉换行符
       
        //发送数据,指明接收端套接字地址
        if(sendto(cfd,buf,sizeof(buf),0,(const sockaddr*)&sin,len)==-1)
        {
            perror("send error");
            return -1;
        }
        printf("send success!\n");

        //我本来就知道他的地址，也不需要定义容器接收地址信息了
        int ret=recvfrom(cfd,buf,sizeof(buf),0,NULL,0);
        if(ret==-1)
        {
            perror("recvfrom error");
            return -1;
        }
        else if(ret==0)
        {
            //printf("对端已下线\n");  没连接呀
            continue;
        }
        buf[ret]=0;
        printf("receive success!buf=%s\n",buf);
        
    }
    
    return 0;
}