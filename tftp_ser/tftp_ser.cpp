#include"tftp_ser.h"
//这就很神奇，ACK<-->数据包  数据部分一定不会被覆盖
//调试打印工具
#define DBG_RECV(n, label) do { \
    printf("[%s] %s: recvfrom 收到 %d 字节, opcode=%d\n", "SER", (label), (n), (n)>0?(int)(unsigned char)msg[1]:-1); \
} while(0)
#define DBG_SEND(n, label) do { \
    printf("[%s] %s: sendto 发 %d 字节\n", "SER", (label), (n)); \
} while(0)
//私有方法
    void tftp_ser::handle_upload() //处理上传请求(2)
    {
        std::string m;
        //接收客户端请求
        printf("[SER] 上传: 接收 WRQ...\n");
        int nrecv=recvfrom(sockfd,msg,MSG_SIZE,0,(sockaddr*)&cin,&clen);
        if(nrecv==-1)
        {
            erorr_log("recvfrom");
        }
        DBG_RECV(nrecv, "上传: 收WRQ");

        //解析上传请求: 文件名和通信模式
        std::string filename= msg+2; //char*直接赋值给string
        const char* cli_conmode=msg+2+filename.size()+1;
        std::cout<<"是否支持上传/下载: (y/n)\n";
        //if(getchar()=='y') setack();
        std::getline(std::cin,m); //接收一行的字符，吞掉换行
        if(m=="y"||m=="Y") setack();
        //判断权限
        //if(!getack()||comode!=cli_conmode)
        if(!getack()||strcasecmp(comode,cli_conmode))
        {
            handle_erorr(2);
            return;
        }

        //说明现在同意上传了

        //创建本地文件来存储客户端文件---就用一个名字
        std::string myfile=filename;
        //myfile="file/"+myfile;
        
       //就算文件存在，这里相当于直接覆盖就行了
       int fd=open(myfile.c_str(),O_WRONLY|O_CREAT|O_TRUNC,0644);
       if(fd==-1) 
       {
            erorr_log("open");
            return;
       }


        //循环发送ACK+接收数据包
        uint16_t block_num=0; //表示接下来我想接收1
        bool finish=0;
        while(1)
        {
            //构造ACK
             msg[1]=4;
             *(uint16_t*)(msg+2)=htons(block_num);
            //发送ACK
            DBG_SEND(4, "上传: 发ACK");
            printf("[SER] 上传: 发 ACK(块号 %d)\n", block_num);
            if(sendto(sockfd,msg,4,0,(const sockaddr*)&cin,clen)==-1)
            {
                erorr_log("sendto");
                return;
            }

            //发完ACK后再退
            if(finish) 
            {
                printf("[SER] 上传完成, 共接收 %d 块\n", block_num+1);
                break;
            }

            //等待客户端响应
            printf("[SER] 上传: 等待 DATA(块号 %d)...\n", block_num+1);
            nrecv=recvfrom(sockfd,msg,MSG_SIZE,0,(sockaddr*)&cin,&clen);
            //MSG_SIZE:最大能接收的字节数，但实际上应该在客户端控制发送量
            if(nrecv==-1)
            {
                erorr_log("recvfrom");
            }
            DBG_RECV(nrecv, "上传: 收DATA");
            
            //解析客户端发来的报文
            if(msg[1]==3)
            {
                //说明是数据包
                //获取他发过来的块编号
                uint16_t cli_num=ntohs(*(uint16_t*)(msg+2));

                //注意：只有当是我想要的编号，我才写入到文件中
                //虽然这在不丢包的理想情况下多此一举，但是如果现实中考虑丢包必须这样写
                if(block_num+1==cli_num) //下一个是我想要的
                {
                    //传输大小应由客户端控制，并且注意写的时候只写数据部分
                    printf("[SER] 上传: DATA块号=%d, 数据 %d 字节, 校验: %s\n",
                        cli_num, nrecv-4, (nrecv-4<512)?"最后一块":"非最后一块");
                    int wret=write(fd,msg+4,nrecv-4);  //写实际读到的字节数
                    if(wret==-1)
                    {
                        erorr_log("write");
                        return ;
                    }

                    if(nrecv-4<512)  //最后一块不足512, 循环出口
                    {
                        finish=1;  //发完最后一个ACK再关呀
                        // printf("[SER] 上传完成, 共接收 %d 块\n", block_num+1);
                        // break;
                    }
                    block_num++; //写了过后再想要下一个
                }
                else
                {
                    printf("[SER] 上传: 块号不匹配! 收到 %d, 期望 %d\n", cli_num, block_num);
                }
            }

            //如果不是数据包，都当成错误处理
            else 
            {
                handle_erorr(0);
                close(fd);
                return;
            }

        }
        close(fd);

    }

    #if 0
    void tftp_ser::handle_download() //处理下载请求(1)
    {
        std::string m;
        if(recvfrom(sockfd,msg,MSG_SIZE,0,(sockaddr*)&cin,&clen)==-1)
        {
            erorr_log("recvfrom");
        }

        //解析下载请求: 文件名和通信模式
        std::string filename= msg+2; //char*直接赋值给string
        const char* cli_conmode=msg+2+filename.size()+1;
        std::cout<<"是否支持上传/下载: (y/n)\n";
       // if(getchar()=='y') setack();
        std::getline(std::cin,m); //读取一行，吞掉换行
         if(m=="y"||m=="Y") setack();
        //判断权限
        if(!getack()||comode!=cli_conmode)
        {
            handle_erorr(2);
            return;
        }

        //说明现在同意下载了,那么文件是否存在？
        //正常来说应该用搜索，但这里走个形式，拼接正确路径，假设存在
        std::string myfile=filename;
        myfile="file/"+myfile;
        int fd=open(myfile.c_str(),O_RDONLY);
        if(fd==-1)
        {
            erorr_log("open");
            return;
        }

        //循环发送数据包和接收ACK
        uint16_t block_num=1; 
        while(1)
        {
            //构造数据包
            msg[1]=3;
            *(uint16_t*)(msg+2)=htons(block_num);
             //考虑丢包的话不能每次都读
            #if 1
                int ret=read(sockfd,msg+4,MSG_SIZE-4); //真正读到的内容
                if(read(sockfd,msg+4,MSG_SIZE-4)==-1)
                {
                    erorr_log("read");
                    return;
                }
            #endif
            //发送数据包   传实际的大小
            if(sendto(sockfd,msg,4+ret,0,(const sockaddr*)&cin,clen)==-1)
            {
                erorr_log("sendto");
                return;
            }

            //等待客户端响应
            if(recvfrom(sockfd,msg,4,0,(sockaddr*)&cin,&clen)==-1)
            {
                erorr_log("recvfrom");
            }

            //解析客户端的报文内容
            if(msg[1]==4)
            {
                //ACK，继续解析块编号
                uint16_t cli_num=ntohs(*(uint16_t*)(msg+2));
                if(cli_num==block_num) 
                {
                    block_num++;
                }

                //那么如果不相等呢？  也就是考虑了重传的情况 所以收发应该再套一个循环

                //循环退出条件：收到了最后一个报文ACK
                if(ret<512)
                {
                    break;
                }
            }
            else //错误报文
            {
                handle_erorr(2);
                close(fd);
                return;
            }
        }
        close(fd);
    }

    #endif
     void tftp_ser::handle_download() //处理下载请求(1)
    {
        std::string m;
        printf("[SER] 下载: 接收 RRQ...\n");
        int nrecv=recvfrom(sockfd,msg,MSG_SIZE,0,(sockaddr*)&cin,&clen);
        if(nrecv==-1)
        {
            erorr_log("recvfrom");
        }
        DBG_RECV(nrecv, "下载: 收RRQ");

        //解析下载请求: 文件名和通信模式
        std::string filename= msg+2; //char*直接赋值给string
        const char* cli_conmode=msg+2+filename.size()+1;
        std::cout<<"是否支持上传/下载: (y/n)\n";
       // if(getchar()=='y') setack();
        std::getline(std::cin,m); //读取一行，吞掉换行
         if(m=="y"||m=="Y") setack();
        //判断权限
        //if(!getack()||comode!=cli_conmode)
        if(!getack()||strcasecmp(comode,cli_conmode)==1)
        {
            
            handle_erorr(2);
            std::cout<<"是你吗,ack="<<getack()<<" comode="<<comode<<" cli="<<cli_conmode;
            return;
        }

        //说明现在同意下载了,那么文件是否存在？
        //正常来说应该用搜索，但这里走个形式，拼接正确路径，假设存在
        std::string myfile=filename;
        //myfile="file/"+myfile;
        int fd=open(myfile.c_str(),O_RDONLY);
        if(fd==-1)
        {
            erorr_log("open");
            return;
        }

        //循环发送数据包和接收ACK
        uint16_t block_num=1;
        
        printf("[SER] 下载: 文件已打开, 开始传输, 共 %d 字节待发送\n", lseek(fd,0,SEEK_END));
        //666,调试的时候把光标弄到最后了
        lseek(fd,0,SEEK_SET);
        while(1)
        {
            //构造数据包
            msg[1]=3;
            *(uint16_t*)(msg+2)=htons(block_num);
            //考虑丢包的话不能每次都读

            printf("[SER] 下载: 准备读第 %d 块 (fd=%d, sockfd=%d)\n", block_num, fd, sockfd);
            int ret=read(fd,msg+4,MSG_SIZE-4); //真正读到的内容
            
        
            printf("[SER] 下载: read 读到 %d 字节\n", ret);
            if(ret==-1)
            {
                erorr_log("read");
                return;
            }
            //考虑一种特殊情况：最后一块刚好是512发过去，那么当发现文件已经没数据了
            //没读到内容就直接退出
            else if(ret==0)
            {
                printf("[SER] 下载: 文件读完, 发送完毕, 共 %d 块\n", block_num-1);
                return;
            }
            while(1)
            {
                //发送数据包   传实际的大小
                DBG_SEND(4+ret, "下载: 发DATA");
                printf("[SER] 下载: 发 DATA(块号 %d, 数据 %d 字节)\n", block_num, ret);
                if(sendto(sockfd,msg,4+ret,0,(const sockaddr*)&cin,clen)==-1)
                {
                    erorr_log("sendto");
                    return;
                }

                //等待客户端响应
                printf("[SER] 下载: 等待 ACK(块号 %d)...\n", block_num);
                int nrecv=recvfrom(sockfd,msg,4,0,(sockaddr*)&cin,&clen);
                if(nrecv==-1)
                {
                    erorr_log("recvfrom");
                }
                DBG_RECV(nrecv, "下载: 收ACK");

                //解析客户端的报文内容
                if(msg[1]==4)
                {
                    //ACK，继续解析块编号
                    uint16_t cli_num=ntohs(*(uint16_t*)(msg+2));
                    printf("[SER] 下载: 收到ACK(块号 %d), 期望 %d\n", cli_num, block_num);
                    //内循环结束条件：发送的ACK正确
                    if(cli_num==block_num)
                    {
                        block_num++;
                        break;
                    }
                    //那么如果不相等呢？  也就是考虑了丢包的情况 所以收发应该再套一个循环
                    //如果是丢包了，就会继续循环发送和接收是否正确，不会去读下一块内容

                    //循环退出条件：收到了最后一个报文ACK

                }
                else //错误报文
                {
                    printf("[SER] 下载: 收到非ACK报文, opcode=%d\n", (int)(unsigned char)msg[1]);
                    handle_erorr(2);
                    close(fd);
                    return;
                }
            }
            //至此说明收到了正确的ACK,外循环退出条件
            if(ret<512)
            {
                printf("[SER] 下载: 最后一块, 传输完成, 共 %d 块\n", block_num-1);
                break;
            }

        }
        close(fd);
    }

    void  tftp_ser::handle_erorr(uint16_t err_code)  //处理错误情况(出错情况很多，直接封装)
    {
        //封装错误报文
        msg[1]=5;
        *(uint16_t*)(msg+2)=htons(err_code);
        int ret=sprintf(msg+4,"错误码是%c",err_code);

        if(sendto(sockfd,msg,ret+4,0,(const sockaddr*)&cin,clen)==-1)
        {
            erorr_log("sendto");
            return;
        }
    }

    //共有方法
     tftp_ser::tftp_ser()
    {
        //this->port=port;

        sockfd=socket(AF_INET,SOCK_DGRAM,0);
        if(sockfd==-1)
        {
            perror("socket error");
            //return;
        }

        sin.sin_family=AF_INET;
        sin.sin_port=htons(this->port);
        sin.sin_addr.s_addr=inet_addr(this->ip);
        slen=sizeof(sin);
        if(bind(sockfd,(const sockaddr*)&sin,slen)==-1)
        {
            perror("bind error");
            //return ;
        }
       
        printf("[SER] bind 成功 -> %s:%d\n",
        inet_ntoa(sin.sin_addr), ntohs(sin.sin_port));

        int reuse =1;
        socklen_t vallen=sizeof(reuse);
        if(setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,(const void*)&reuse,vallen)==-1)
        {
            erorr_log("setsockopt error");
            return;
        }
    }
     tftp_ser::~tftp_ser()
     {
        close(sockfd);
     }

    void tftp_ser::run()  //运行服务器
    {
        //循环，不然一轮服务器就下线了
        while(1)
        {
            //等待客户端发来请求，并看一看是什么消息
            printf("[SER] 等待客户端请求 (peek)...\n");
            int npeek=recvfrom(sockfd,msg,MSG_SIZE,MSG_PEEK,(sockaddr*)&cin,&clen);
            if(npeek==-1)
            {
                erorr_log("recvfrom");
            }
            else
            {
                printf("[SER] PEEK: 收到 %d 字节, opcode=%d, 来自 %s:%d\n",
                    npeek, (int)(unsigned char)msg[1],
                    inet_ntoa(cin.sin_addr), ntohs(cin.sin_port));
            }

            int select=msg[1];
            switch (select)
            {
            case 1:
                printf("执行下载操作\n");
                handle_download();
                break;
            case 2:
                printf("执行上传操作\n");
                handle_upload();
                break;
            
            //不需要，客户端自己下线就行了
            // case 3:
            //     printf("客户端[%s:%d]退出\n",inet_ntoa(cin.sin_addr),ntohs(cin.sin_port));
            //     break;
            default:
                printf("请求错误\n");
                handle_erorr(2);
                break;
            }
            setack();
        }
    }


    void tftp_ser::setack()
    {
        ack=1-ack;
    }
    bool tftp_ser::getack()
    {
        return ack;
    }