//g++ chatser.cpp main.cpp -pthread -std=c++11
//./a.out 172.17.165.43 5555
#include"chatser.h"
void chatser::startserver(const char*ip,uint16_t port) //初始化服务器
{
     //1.创建套接字(信息的载体)并返回它的文件描述符
    sfd=-1;
    if((sfd=socket(AF_INET,SOCK_STREAM,0))==-1)  
    //地址族因特网(ipv4), 流式(tcp), 默认协议
    {
        error_log("socket error");
    }


    printf("socket create success!sfd=%d\n",sfd);

     int reuse =1;
    socklen_t vallen=sizeof(reuse);
    if(setsockopt(sfd,SOL_SOCKET,SO_REUSEADDR,(const void*)&reuse,vallen)==-1)
    {
         error_log("setsockopt error");
    }

    //2.绑定-将IP地址和端口号与套接字绑定，以使别人能找到我

        //2.1 填充要绑定的ip地址,端口号,地址族结构体 。  _in是ipv4专属
    struct sockaddr_in sin;
    sin.sin_family=AF_INET;
    sin.sin_addr.s_addr=inet_addr(ip);  //wtm结构体里面是结构体来着

    sin.sin_port=htons(port); //传输16位整数嘛

        //2.2填嘛，细节太多了
    if(bind(sfd,(const sockaddr*)&sin,sizeof(sin))==-1)
    {
        error_log("bind error");
    }
     std::cout<<"绑定成功\n";

    //3.套接字启动监听模式(监听)
    if(listen(sfd,128)==-1)
    // 128:已连接队列的最大长度
    //也就是说，最多允许有 128 个已经连好的客户端在后台排队等 accept()
    {
        error_log("listen error");
    }
    printf("listen success!\n");
}


void chatser::startpool(int poolsize)  //初始化线程池
{
    //工作线程数量
    for(int i=1;i<=poolsize;i++)
    {
        worker.emplace_back([this](){  //lambda原地构造线程执行逻辑
            while(1)
            {
                std::function<void()> f;
                {
                    //先抢锁
                    std::unique_lock<std::mutex>lock(m);

                    cv.wait(lock,[this](){ //再写个lambda判断是否睡觉 循环
                        if(this->stop==false&&tasks.empty()) return 0; //阻塞
                        else return 1;  //不阻塞
                    });

                    //拿到锁了，有两种情况
                    //退出
                    if(tasks.empty()&&stop==true) 
                    {
                        std::cout<<std::this_thread::get_id<<"顺利结束\n";
                        return;
                    }

                    //工作
                    f=tasks.front();
                    tasks.pop();
                }//至此，拿到任务了(tasks被修改) 可以释放锁，线程执行任务不需要锁，因为它的工作内容不重要

                f(); //执行任务
                //当执行完了又回去
            }
        });
    }
}

void chatser::addtask(std::function<void()>f)  //添加任务
{
    //修改任务数量，需要保护
    {
        std::unique_lock<std::mutex>lock(m);
        tasks.push(f);
    }
    //添加一个，尝试唤醒一个
    cv.notify_one();
}

void chatser::handletask(const int&client_fd,sockaddr_in cin)  //处理任务
{
    //定义消息容器接收客户端发来的消息
    MSG recv_buf;
    char recv_str[sizeof(MSG)]; //发过来的是二进制  难道不是字节？
    while(1)
    {
        int recv_num=recv(client_fd,recv_str,sizeof(MSG),0);
        // if(recv_num==-1)
        // {
        //     error_log("recv error");
        //     break;
        // } 

        // else if(recv_num==0)  //连接的两方，一方下线过后另一方会返回0
        // {
        //     std::cout<<"对端异常退出\n"
        // }

        //都需要下线，那么合并到一起
        if(recv_num<=0)
        {
            // 遍历客户端容器
            auto it = client_set.begin(); // 定义遍历客户端容器的迭代器
            while (it != client_set.end())
            {
                if (it->client_fd == client_fd) // 在容器中找到当前套接字
                {
                    client_set.erase(it); // 将当前套接字从列表中移除
                    break;
                }
                ++it; // 指针向后偏移
            }
            close(client_fd); // 关闭套接字
            break;
        }



        //开始解析发来的消息吧
       // recv_buf.rexuliehua(std::string(recv_str)); //匿名构造
      recv_buf.deserialize(std::string(recv_str, recv_num));  // ← 必须带 recv_num 
       
       int type=recv_buf.type;
        char*name=recv_buf.cliname;
        char* text=recv_buf.text;
        //解析成功，进行后续判断
        switch (type)
        {
        case LOGIN:
            {
                std::unique_lock<std::mutex> lock(m); //client.set也需要被保护

                struct client newclient{cin,client_fd};
                client_set.emplace_back(newclient);
                //广播遍历所有的客户端，告诉它们谁登录成功了。
                //先构造消息
                sprintf(recv_buf.text,"*****%s登录*****\n",recv_buf.cliname);
                
                broadcast(recv_buf,-1); //发给所有的，包括自己
                printf("login success!\n");
                break;
            }
            
        
        case CHAT:
            //一个客户端与另一个客户端聊天，借助服务器转发，那么消息必须有想要发送的
            //ip和端口，以及更多自己的信息。但是这里简单的认为是广播
            broadcast(recv_buf,client_fd);
            printf("chat success!\n");
            break;

        case QUIT:
            //退出。通知所有的客户端谁要退出
               //先构造消息
            sprintf(recv_buf.text,"*****%s退出*****\n",recv_buf.cliname);
            broadcast(recv_buf,-1);

            for(auto i=client_set.begin();i!=client_set.end();i++)
            {
                if(i->client_fd==client_fd) 
                {
                    client_set.erase(i);
                    break;
                }
                i++;
            }

            printf("quit success!\n");
            
            close(client_fd);
            return; //线程也循环！

        default:  //咋通知客户端
            //std::cout<<"解析失败，请重新发送\n";  //改用send，处理错误
            
            
            break;
        }
    }
    
}

//初始化工作
chatser::chatser(int poolsize,const char*ip,uint16_t port):stop(false)
{
    startserver(ip,port);
    startpool(poolsize);
}

//优雅结束
chatser::~chatser()
{
    {
        //实现原子操作
        std::unique_lock<std::mutex>lock(m);
        stop=true;
    }

    cv.notify_all(); //唤醒所有在睡觉的线程

    for(auto&i:worker)
    {
        i.join();
    }
    close(sfd);
}

//启动服务器  具体：等待连接和执行处理任务函数
void chatser::run() 
{
    //定义容器接收发来连接请求的客户端套接字地址信息，以及套接字文件描述符
    while(1)
    {
        sockaddr_in cin;
        socklen_t clen=sizeof(cin);
        int client_fd=-1;
        //client_fd虽然本质上服务器端的新套接字，通信套接字，
        //但是它仅可以与特定的客户端通信，所以这里可以代表客户端套接字，但不是真的客户端套接字
        client_fd=accept(sfd,(sockaddr*)&cin,&clen);
        if(client_fd==-1)
        {
            error_log("accept error");
            return;
        }
        std::cout<<"accept success! ip="<<inet_ntoa(cin.sin_addr)<<" port="<<ntohs(cin.sin_port)<<"\n";


        //执行处理任务函数  与客户端通信就是一个任务体
        //addtask(handletask);  不行，handletask是类的方法，需要this指针，func必须传入this指针变量

        //所以可以用lambda捕获this后在调用这个函数
        addtask([this,client_fd,cin](){
            this->handletask(client_fd,cin);  //内部传参，但外部是无参的！
        });
    }
}

void chatser::error_log(const char* k)
{
    perror(k);
    std::cerr<<"\n"<<__LINE__<<" "<<__func__<<" "<<__FILE__<<"\n";
}


void chatser::broadcast(MSG&msg,int exclude_fd)
{
    //先序列化转变为字符串
    msg.serialize();
    for(auto&i:client_set)
    {
        if(i.client_fd!=exclude_fd)
        {
            if(send(i.client_fd,(const void*)&msg,sizeof(msg),0)==-1)
            {
                error_log("send error");
            }
        }
    }
}