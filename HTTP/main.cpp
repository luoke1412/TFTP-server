//演示http服务器从接收请求-->解析请求--->响应请求
#include "http.h"
using namespace std;
int HTTP_PORT=80; //默认端口号
char HTTP_IP[128]= "172.17.165.43";

struct Info
{
    sockaddr_in cin; //客户端套接字信息
    int childfd;  //子线程的套接字文件描述符
};

void* MSG_request(void* arg) //任务：接收,解析，响应客户端信息
{
    sockaddr_in cin=((Info*)arg)->cin;
    int childfd=((Info*)arg)->childfd;

   // char ip[128]=inet_ntoa(cin->sin_addr);  不能用指针直接给数组赋值
   char*ip=inet_ntoa(cin.sin_addr);
    int port=ntohs(cin.sin_port);
    

    //调用消息处理函数，该函数是所有请求的入口
    if(handle_msg(childfd)==-1)
    {
        printf("handle_msg error!\n");
       // return NULL;
    }
    /*  test
    char buf[4096];
    while(1)
    {
        bzero(buf,sizeof(buf));
        int ret=recv(childfd,buf,sizeof(buf),0);
        if(ret==-1)
        {
            perror("recv erorr");
            continue;
        }

        else if(ret==0)
        {
            printf("对端[%s:%d]已下线\n",ip,port);
            break;
        }

        buf[ret]=0;
        printf("recv success,buf=%s\n",buf);
    }
    pthread_exit(NULL); //退出线程
    */

    close(childfd);
    return NULL;
}


/************************************** */
//主函数，框架
int main(int argc, const char *argv[])
{
    //拓展1(外部传入端口号)
   // strcpy(HTTP_IP,"172.17.165.43");

    HTTP_PORT=80;
    if(argc>1)
    {
        HTTP_PORT= atoi(argv[1]);  //字符串转化为数字
    }
    //printf("port=%d",HTTP_PORT);
    
    //写在初始化里面
     //拓展2(每次断开还会继续占用地址，所以修改成可以地址快速重用属性)
    // int reuse =1;
    // socklen_t vallen=reuse;
    // setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,(const void*)&reuse,vallen);

    //1.初始化服务器,并返回套接字描述符(包括socket,bind,listen)
    int sockfd=Init_Http();
    if(sockfd==-1)
    {
        printf("Init_HTTP error!\n");
        return -1;
    }

    


    //2.并发接收请求，这里以多线程为例
        //主线程负责监听到请求并创建子线程套接字，子线程负责通信
    while(1)
    {
        Info info;
        //接收来自客户端套接字的信息
        sockaddr_in cin;
        socklen_t socklen=sizeof(cin);
        int childfd=accept(sockfd,(sockaddr*)&cin,&socklen);
        if(childfd==-1)
        {
            perror("accept error");
            continue;
        }

        info.childfd=childfd;
        info.cin=cin;
        //至此，建立连接成功
        pthread_t pid=-1;
        if(pthread_create(&pid,NULL,MSG_request,(void*)&info)==-1)
        {
            printf("pthread_create error!\n");
            continue;
        }

        pthread_detach(pid); //分离态，系统回收子线程资源
    }
    //close(pid);
    close(sockfd);
    return 0;
}