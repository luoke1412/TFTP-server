#include"chatcli.h"

chatcli::chatcli(const char* ip,uint16_t port)
{
    startclient(ip,port);
}

chatcli::~chatcli() 
{
    close(cfd);
}

void chatcli::startclient(const char* ip,uint16_t port)  //初始化客户端
{
    //1.创建套接字
    cfd=socket(AF_INET,SOCK_STREAM,0);
    if(cfd==-1)
    {
        perror("socket error");
        return;
    }
    std::cout<<"socket success!\n";

    //先设置地址重用属性
    int reuse=1;
    if(setsockopt(cfd,SOL_SOCKET,SO_REUSEADDR,(const void*)(&reuse),sizeof(reuse))==-1)
    {
        perror("setsockopt error");
        return;
    }

    //绑定地址
    cin.sin_family=AF_INET;
    cin.sin_addr.s_addr=inet_addr(ip);
    cin.sin_port=htons(port);
    clen=sizeof(cin);
    if(bind(cfd,(const sockaddr*)&cin,clen)==-1)
    {
        perror("bind error");
        return;
    }
    std::cout<<"bind success!\n";

    
    std::cout<<"setsockopt success!\n";
    
}

//发送消息： 构造结构体--->string--->字符串
void chatcli::Sendmsg() //封装发送消息方法--->主线程
{

    while(running)
    {
        printf("请在这里输入发送的消息:(2.CHAT 3.QUIT) ");
        //从stream指向的文件中最多读取size-1个字符到s容器中,再多都会截断
        std::cin>>mymsg.type;
        getchar();  //吞掉换行符 
        printf("请在这里输入发送的消息正文:");
        fgets(mymsg.text,TEXT,stdin);
        mymsg.text[TEXT]=0;
        
        std::string mymsg_string=mymsg.serialize();

        int ret=send(cfd,mymsg_string.c_str(),sizeof(mymsg_string.c_str()),0);
        if(ret==-1)
        {
            perror("send error");
            return;
        }

        //再来判断是不是退出请求
        if(mymsg.type==QUIT)
        {
            running=false; //其实不需要...
            std::cout<<"退出成功~\n";
            break;
        }
    }
    //exit(EXIT_SUCCESS); //把整个进程都退出，子线程自然也会退出
}


//接收消息： 接收字符串-->string--->结构体
void chatcli::Recvmsg() //封装接收消息方法--->子线程
{
    char buf[TEXT*2]; //接收消息的容器
    MSG recv_msg;  //存储反序列的数据
    while(running)
    {
        int ret=recv(cfd,buf,sizeof(buf),0);
        if(ret==-1)
        {
            perror("recv error");
        }

        //反序列化???我接受的明明是字符串，但反序列化却是string。。。
        //没得问题
        //构造成string
        std::string recv_msg_string(buf,ret);

        //反序列化
        recv_msg.reserialize(recv_msg_string);

        //此时转化为了结构体
        //其他客户端不需要管消息的类型，只需要读消息的正文(被服务器封装好的)就行了
        printf("收到消息：%s\n",recv_msg.text);
    }
}


void chatcli::run(const char* cliname) //运行客户端
{
    running=true;
    //开启连接模式
    sockaddr_in sin;
    sin.sin_family=AF_INET;
    sin.sin_addr.s_addr=inet_addr(SER_IP);
    sin.sin_port=htons(SER_PORT);
    if(connect(cfd,(const sockaddr*)&sin,sizeof(sin))==-1)
    {
        perror("connect error");
        return;
    }
    std::cout<<"connect success!\n";

    //至此连接成功
    
    //必须先登录，并发送登录消息
    //构造登录消息
    //struct MSG mymsg;  放类成员去了
    std::string mymsg_string; //存储字节序消息
    mymsg.type=LOGIN;
    strncpy(mymsg.name,cliname,sizeof(mymsg.name)-1); //超过就截断
    //不需要正文
    //序列化转为二进制发送
    mymsg_string=mymsg.serialize();
    int ret=send(cfd,mymsg_string.c_str(),sizeof(mymsg_string.c_str()),0);
    if(ret==-1)
    {
        perror("send error");
        return;
    }

    //至此，已成功登录
    std::cout<<"send login success!\n";

    //创建一个子线程用于接收消息
    //class std::thread th(&chatcli::Recvmsg,&object);  //.h才叫类里面，.cpp都是类外面 。然后还需要对象指针

    //我将竭尽毕生所学研究lambda表达式
    class std::thread th([this]() 
        { 
            Recvmsg(); 
        });

    //主线程继续阻塞发送消息
    Sendmsg();

    th.join(); //阻塞回收线程资源
}   


