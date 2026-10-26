#pragma once
#include"../myhead.h"
#define erorr_log(msg) do{\ 
    perror(msg);\
    std::cout<<__LINE__<<" "<<"__func__"<<" "<<__FILE__<<"\n";\
} while (0)
 //不写数据类型，只是机械的替换文本

#define SER_IP "172.18.0.1"
#define SER_PORT 69  //这个是默认吧
#define MSG_SIZE 516

class tftp_ser
{
    private:
    int sockfd;  //套接字文件描述符
    uint16_t port=SER_PORT;  //端口号,可变
    const char* ip=SER_IP;
    sockaddr_in sin;  //服务器套接字地址
    socklen_t slen;

    char msg[MSG_SIZE];  //发送和接收数据的容器
    sockaddr_in cin;  //接收客户端套接字的地址也很有必要
    socklen_t clen;
    bool ack=0; //是否支持上传/发送文件，默认不支持
    const char* comode="octet";


    //私有方法(只能类内访问，类外不可访问。  所以只能通过run来调用)
    void handle_upload(); //处理上传请求

    void handle_download(); //处理下载请求

    void handle_erorr(uint16_t err_code);  //处理错误情况(出错情况很多，直接封装)

    void setack();
    bool getack();

    //共有方法
    public:
    tftp_ser();  //初始化服务器

    ~tftp_ser(); //回收资源

    void run();  //运行服务器
};
