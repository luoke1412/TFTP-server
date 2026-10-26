#pragma once
#include"../../myhead.h"
#include<iostream>
#include<thread>
#include<condition_variable>
#include<mutex>
#include<functional>
#include<queue>
#define LOGIN 1
#define CHAT 2
#define QUIT 3
#define TEXT 128
#define SER_IP "172.17.165.43"
#define SER_PORT 5555
class chatcli
{
    
public: //先定义消息数据类型
    struct MSG
    {
        int type;
        char name[20];
        char text[TEXT];

        //定义序列化和反序列化函数
        //从结构体到二进制，传输时使用
        std::string serialize()
        {
            std::string temp;
            temp.append(reinterpret_cast<const char*>(&type),sizeof(type));
            temp.append(name,sizeof(name));
            temp.append(text,TEXT);
            return temp;
        }

        //二进制到结构体，解析时使用
        void reserialize(std::string&m)
        {
            //strncpy()  逐字符拷贝
            int off=0;
            memcpy(&type,m.c_str()+off,sizeof(int)); //逐字节拷贝，不关心是什么数据类型
            off+=sizeof(int);
            type=ntohl(type);

            memcpy(name,m.c_str()+off,sizeof(name));
            off+=sizeof(name);

            memcpy(text,m.c_str()+off,sizeof(text));

        }
    };

private:
    //与通信相关
    int cfd;
    sockaddr_in cin;
    socklen_t clen;
    
    MSG mymsg; //定义发送消息的结构体变量
    
    //定义一个标志表示是否启动
    bool running;  

    
    //私有方法

    void startclient(const char* ip,uint16_t port);  //初始化客户端
    void Sendmsg(); //封装发送消息方法--->主线程
    void Recvmsg(); //封装接收消息方法--->子线程
public:
    chatcli(const char* ip,uint16_t port);
    ~chatcli();

    void run(const char* cliname); //启动客户端
};






