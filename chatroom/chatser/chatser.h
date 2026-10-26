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
#define TEXT 128  //正文最大长度
class chatser
{
public:  //先定义数据类型

    //自定义消息结构体类型
    struct MSG  
    {
        int type;  //消息类型
        char cliname[20]; //客户端名字
        char text[TEXT];  //消息正文

        //这里再补充两个方法，用于提高传输效率

        //序列化：将结构体转化为字符串
        std::string serialize() const
        {
             std::string m="";       //传地址，强转成char*
            m.append(reinterpret_cast<const char*>(&type),sizeof(type)); 
            m.append(cliname,sizeof(cliname));
            m.append(text,sizeof(text));  //已经是char*类型，不用强转

            return m;
        }

        //反序列化：将二进制(你接收到的)转化为正常数据类型
        void deserialize(std::string recv_cli)
        {
            //解析string
            //memcpy((void*)&type,recv_cli.c_str(),sizeof(int)); // n bytes
            
            int off=0;  //偏移量
            
            memcpy((void*)&type,recv_cli.c_str()+off,sizeof(int));
            off+=sizeof(int);
            type=ntohs(type);

            memcpy(cliname,recv_cli.c_str()+off,sizeof(cliname));
            off+=sizeof(cliname);

            memcpy(text,recv_cli.c_str()+off,sizeof(text));

        }
    };

    //客户端信息结构体
    struct client
    {
        sockaddr_in cin; 
        int client_fd;  //给通信用的 
        //char cliname[20]; //客户端名字，给其他客户端看的  服务器拼接就行了
    };

    //哈哈，其实只需要数组存储client_fd就行了，char cliname[20]发来的msg知道，sockaddr_in cin用不上


private:
   //通信相关
   int sfd;
   sockaddr_in sin;  //客户端套接字地址信息
   socklen_t slen;  //长度

   //线程池相关
   std::vector<std::thread> worker;
   std::queue<std::function<void()>> tasks;
   bool stop=false;
    //那么客户端/客户端信息呢？  不需要，我已经创建了客户端的任务队列
    //客户端信息在accept的时候就能获取，作为参数传递，不需要再单独存储

    
    //与同步与互斥有关
    std::mutex m;
    std::condition_variable cv;
    
    //存储在线的客户端
    std::vector<client>client_set;

    //私有方法
    void startserver(const char*ip,uint16_t port); //初始化服务器
    void startpool(int poolsize);  //初始化线程池
    void addtask(std::function<void()>f);  //添加任务  参数：无参、无返回值的函数
    void handletask(const int&newfd,sockaddr_in cin);  //处理任务  为啥穿不了引用
    void error_log(const char* k);
    void broadcast(MSG&msg,int exclude_fd); //给除了fd的其他客户端广播内容
public:
    chatser(int poolsize,const char*ip,uint16_t port);
    ~chatser();

    void run(); //入口，等待连接和执行处理任务函数
};






