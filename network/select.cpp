/*
演示一下select的作用：  把两个文件描述符newfd,sfd都装进集合里
然后触发那个就执行哪个， 具体来讲  如果有连接，就sfd工作   如果有通信就newfd工作
*/
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

/*******************select部分*********************** */
//1.把现有的文件描述符放进集合里
fd_set readfds;
//先清空
FD_ZERO(&readfds); 

FD_SET(sfd,&readfds);

//演示服务器端主动发送数据
FD_SET(0,&readfds);


int maxfd=sfd;  //暂时存储最大的描述符值

 sockaddr_in addr_arr[1024]; //见下，一个集合最大容量为1024
while(1)
{

    fd_set tempfds=readfds;

    int res=select(maxfd+1,&tempfds,NULL,NULL,NULL);  //返回被触发的函数个数

    if(res==-1)
    {
        perror("select error");
        continue;
        //return -1;
    }
    else if(res==0)
    {
        printf("time out!\n");
        break;
    }

    //至此，说明集合中至少有一个事件被触发过，可以直接进行判断
    
    
/*
通信这里，改动还蛮大的.
(1)首先收发数据不能用循环，因为不是多线程/多进程，就卡在一个客户端里面了，况且外面有循环
(2)然后又有一个问题，下线在哪关闭套接字咧  对端下线的时候
(3)呀，还有一个问题，就是cin啊。  cin是客户端套接字地址信息，是sdf中获取的，同时也有一个相应的和他通信的newfd
那么怎么确认我的这个newfd就是我想要的cin呢？
就是映射呀，newfd(整数)---cin  用数组提前存储就好了
*/

    //sfd=3，那么他创建出来的其他描述符一定是>3的，遍历即可
    //不，别写死了，不一定是3，就用sfd表示
    for(int i=maxfd;i>sfd;i--)
    {
        if(FD_ISSET(i,&tempfds))
        {
            //5.收发数据
        //5.1 定义容器接收数据
        char buf[128];
        //循环接收和发送
        //while(1)
    
        bzero(buf,sizeof(buf));
        res=recv(i,buf,sizeof(buf),0); //以阻塞的形式接收数据
        if(res==-1)
        {
            perror("recv error");
            return -1;
        }
        else if(res==0)
        {
            printf("对端已下机\n");

            close(i);
            //break;

            //哦~，这个客户下线之后记得更新maxfd
            if(i==maxfd)
            {
                for(int j=i-1;;j--)
                {
                    if(FD_ISSET(j,&readfds))
                    {
                        maxfd=j;
                        break;
                    }
                }
            }

            //既然i已经退出了，也要从read集合中把它移除。尽管select会把没有事件的描述符给过滤掉
            //但它还是会在readdfs中占用描述符，占用空间啊
            FD_CLR(i,&readfds);
        }
        buf[res] = '\0';

        printf("recv [%s:%d]%s\n",inet_ntoa(addr_arr[i].sin_addr),ntohs(addr_arr[i].sin_port),buf);

        //发过去
        
        if(send(i,buf,strlen(buf),0)==-1) //0:阻塞等待(别人容器满了)
        {
            perror("send error");
            return -1;
        }
        printf("send success\n");

    
        //6.关闭套接字(两个都要)
        //close(sfd); //这个不要关哦，还要和其他客户端连接
        }
    }
        
    if(FD_ISSET(sfd,&tempfds))
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
        
        //创建了新的描述符，别忘了把它也加入到集合中去,同时更新一下maxfd呗
        FD_SET(newfd,&readfds);
        maxfd=max(maxfd,newfd); //newfd不一定是最大的，万一较小的突然下线，这次恰好又把它分配给newfd了呢
    
        //当然，还要把客户端套接字信息存储下来
        addr_arr[newfd]=cin; 
    }    

    if(FD_ISSET(0,&tempfds))
    {
        char buf[128]="";
        fgets(buf,sizeof(buf),stdin);
        buf[strlen(buf)-1]=0;
        //send(newfd,buf,sizeof(buf),0);
        //找不到要发送的指定的客户端，那就发给所有还在的客户端
        for(int i=sfd+1;i<=maxfd;i++)
        {
            if(FD_ISSET(i,&readfds))
            send(i,buf,strlen(buf),0); //发送实际长度
        }
    }


}
    close(sfd); 

    return 0;
}