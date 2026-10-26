//这就很神奇，ACK<-->数据包  数据部分一定不会被覆盖，所以说重传是很容易的一件事
#include "tftp_cli.h"

//调试打印工具
#define DBG_RECV(n, label) do { \
    printf("[%s] %s: recvfrom 收到 %d 字节, opcode=%d\n", "CLI", (label), (n), (n)>0?(int)(unsigned char)msg[1]:-1); \
} while(0)
#define DBG_SEND(n, label) do { \
    printf("[%s] %s: sendto 发 %d 字节\n", "CLI", (label), (n)); \
} while(0)

//下载
/*
1.封装一个读请求报文(包含操作码，你要的文件名,通信模式等)  同时想到我们是不是也可以把报文数组作为成员
                                        更甚者 四种报文，只要转化得当，我们可以共用一个容器
1.5创建一个文件用于存储从服务器端下载的文件
2.发送给服务器
3.等待响应(接收返回来的消息)
4.循环接收服务器发过来的数据包
5.根据收到的数据包的块编号，判断是不是自己想要的，然后组装发送下一个ACK包
6.当发现数据量小于512B时，结束
*/
//呜呜，不准用strlen()，因为msg第一位就是0。就用返回值，有多少读多少，有多少写多少
 void tftp_cli::DoDownLoad()
{
    //std::cout<<"DoUpLoad\n";
    std::cout<<"请输入你要下载的文件名字:";
    std::getline(std::cin,filename);

    //组装写请求报文
    int ret=sprintf(msg,"%c%c%s%c%s%c",0,1,filename.c_str(),0,"octet",0); //netascii只能传输文本
    #if 0
    printf("msg=%s,strlen=%d\n",msg,strlen(msg));    666字符串长度是0啊！
   #endif

    // if(msg[1]==1) std::cout<<"hahaha";  打印出来了
    
    //将第三部分参数保留第二部分参数的形式，转化为字符串填充到容器中
    //%s只会显示字符串本身，不会显示终止符

    //发送过去
    DBG_SEND(ret, "下载: 发RRQ");
    if(sendto(sockfd,(const void*)msg,ret,0,(const sockaddr*)&sin,slen)==-1)
    {
        perror("sendto error");
        return;
    }
    printf("[CLI] 下载: RRQ 已发出, 等待服务器响应...\n");

    //打开文件，准备存储
    std::string myfile="file/"+filename;
    int fd=open(myfile.c_str(),O_WRONLY|O_CREAT|O_TRUNC,0664);//解释一下：以只写的方式打开，没有的话就创建
    //这没问题。那为什么每次打开都要清空呢？  文件传输，肯定是要求一次性全部成功的接收到所有数据
    //那万一传了一半，出错了结束传输。只能重新执行下载操作，文件里缓存的数据必然会影响下一次传输
    if(fd==-1)
    {
        perror("open error");
        return;
    }
    //等待服务器响应
    //如果服务器文件不存在/访问违规等情况会收到ERROR
    //char buf[MSG_SIZE];
    //来，我们的msg负责发送，buf负责接收 no


    unsigned short num=0;  //块编号，从1开始递增  num:我已经接收到的  num+1 我想要的
    while(1)
    {
        printf("[CLI] 下载: 等待第 %d 块 DATA...\n", num+1);
        //等待服务器响应
        if((ret=recvfrom(sockfd,msg,MSG_SIZE,0,(sockaddr*)&sin,&slen))==-1)
        {
            perror("recvfrom error");
            close(fd);
            return;
        }
        DBG_RECV(ret, "下载");

        //如果是数据包
        if(msg[1]==3)
        {
            //解析数据包
            uint16_t ser_num=ntohs(*(uint16_t*)(msg+2));
            //解释：(msg+2)是首地址，
            //(unsigned short*)表示偏移量是short个单位，  *是取地址存储的值
            //因为是作为网络字节序传输的，所以要转化为主机字节序存储

            printf("[CLI] 下载: 收到块号=%d, 数据量=%d 字节, 我期望=%d\n", ser_num, ret-4, num+1);

            if(num+1==ser_num) // num+1才是我想要的
            {
                num++;
                //并且在这里读取新内容
                ret=write(fd,msg+4,ret-4); //有多少写多少
                if(ret==-1)
                {
                    perror("write error");
                    close(fd);
                    return;
                }

            }
            else
            {
                printf("[CLI] 下载: 块号不匹配! 收到 %d, 期望 %d (重复包, 不写文件, 只重发ACK)\n", ser_num, num+1);
            }
        }

        else //错误处理
        {
            printf("[CLI] 下载: 收到非DATA报文, opcode=%d, num=%d\n", (int)(unsigned char)msg[1], num);
            uint16_t err_code = ntohs(*(uint16_t*)(msg+2));
            printf("ERROR message:%d!\n",err_code);
            //std::cout<<"num="<<num<<std::endl;  num=1
            close(fd);
            return;
        }

         //组装ACK发过去
        msg[1]=4,*(unsigned short*)(msg+2)=htons(num);
        DBG_SEND(4, "下载: 发ACK");

        if(sendto(sockfd,msg,4,0,(const sockaddr*)&sin,slen)==-1)
        {
                perror("sendto error");
                close(fd);
                return;
        }
        printf("[CLI] 下载: 已发 ACK(块号 %d)\n", num);

        //判断数据量是否是512
        if(ret!=512)
        {
            printf("[CLI] 下载: 最后一块 (%d 字节), 传输结束\n", ret);
            break;
        }
    }

    printf("[CLI] 下载完成, 共接收 %d 块\n", num);
    close(fd);
} 



 //上传
 /*
 1.检查自己本地有没有这个文件，没有的话直接退出
 2.封装一个写请求
 3.发送写请求
 4.等待服务器响应
 5.如果是ACK，封装成数据包
 6.循环发送数据包和接收服务器的ACK
 7.如果发现自己发送的数据量小于512并且呢也收到ACK(保证最后一个数据包也收到了) 就退出循环
 */
//不要用strlen()，因为第一位就是0。最好用返回值，或者把首地址改对用strlen
 void tftp_cli::DoUpLoad()
 {
     std::cout<<"请输入你要上传的文件名";
     std::cin>>filename; 
     getchar();  //吞一下换行符，所以就说cin,getline不能交替使用

     //接下来就是寻找有没有这个文件
     //拼接目录(更底层的应该是用DFS搜索本目录下的文件)
    std::string fullname="file/"+filename;

    int fd=open(fullname.c_str(),O_RDONLY);
    if(fd==-1)
    {
        perror("open error");
        return;
    }

    //封装一个写请求
    int ret=sprintf(msg,"%c%c%s%c%s%c",0,2,filename.c_str(),0,"octet",0);

    //发送写请求
    DBG_SEND(ret, "上传: 发WRQ");
    if(sendto(sockfd,msg,ret,0,(const sockaddr*)&sin,slen)==-1)
    {
        perror("sendto error");
        close(fd);
        return;
    }
    printf("[CLI] 上传: WRQ 已发出, 等待服务器 ACK(块号0)...\n");


    unsigned short num=0;   //块编号，虽然是从1开始递增，但是第一个ACK还没发送数据，所以ACK编号是0
    //他也是0
    while(1)
    {
        //等待服务器响应
        printf("[CLI] 上传: 等待 ACK(块号 %d)...\n", num);
        int nrecv=recvfrom(sockfd,msg,4,0,(sockaddr*)&sin,&slen);
        if(nrecv==-1)
        {
            perror("recvfrom error");
            break;
        }
        DBG_RECV(nrecv, "上传");

         //判断是否是ACK啦
        if(msg[1]==4)
        {
            //ACK 解析块编号
            uint16_t ser_num=ntohs(*(uint16_t*)(msg+2));
            printf("[CLI] 上传: 收到ACK(块号=%d), 我期望=%d\n", ser_num, num);
            if(ser_num==num)
            {
                num++;
                //可以读新内容啦
                ret=read(fd,msg+4,MSG_SIZE-4);
                printf("[CLI] 上传: read 读到 %d 字节\n", ret);
                if(ret==-1)
                {
                    perror("read error");
                    break;
                }

                else if(ret==0)
                {
                    std::cout<<"上传成功\n";
                    break;
                }
            }
            else
            {
                printf("[CLI] 上传: ACK块号不匹配! 收到 %d, 期望 %d\n", ser_num, num);
            }
        }

        else //处理错误情况
        {
            uint16_t err_code = ntohs(*(uint16_t*)(msg+2));
            printf("[CLI] 上传: 收到非ACK报文, opcode=%d, 错误码=%d\n", (int)(unsigned char)msg[1], err_code);
            break;
        }
        //现在是ACK啦,看他目前收到的块编号


        //继续封装成数据包
         msg[1]=3;
        *(uint16_t*)(msg+2)=htons(num);
        DBG_SEND(ret+4, "上传: 发DATA");

        //发送数据包到服务器
        if(sendto(sockfd,msg,ret+4,0,(const sockaddr*)&sin,slen)==-1)
        {
            perror("sendto error");
            close(fd);
            return;
        }
        printf("[CLI] 上传: 已发 DATA(块号 %d, 数据 %d 字节)\n", num, ret);

        // if(ret<512) //nonono,不仅要发送完数据，还要收到ACK才算结束
        // {
        //     break;
        // }
        //那我何不直接判断读到的内容是否为0

    }
    close(fd);
 }
 /*
  如果num不对就退出码？  就不可以设计一个循环发送数据包，直到对方发送ACK吗
 */

//初始化
tftp_cli::tftp_cli(short port)
{
    this->port=port;

    sockfd=socket(AF_INET,SOCK_DGRAM,0);
    if(sockfd==-1)
    {
        perror("socket error");
        //return;
    }

    sockaddr_in cin;
    cin.sin_family=AF_INET;
    cin.sin_port=htons(this->port);
    cin.sin_addr.s_addr=inet_addr(this->ip);
    socklen_t len=sizeof(cin);
    if(bind(sockfd,(const sockaddr*)&cin,len)==-1)
    {
        perror("bind error");
        //return ;
    }

     printf("bind success!\n");

    //同时也初始化一下服务器的套接字地址
    sin.sin_family=AF_INET;
    sin.sin_port=htons(SER_PORT);
    sin.sin_addr.s_addr=inet_addr(SER_IP);

    printf("[CLI] 发往 -> %s:%d\n",
       inet_ntoa(sin.sin_addr), ntohs(sin.sin_port));

    slen=sizeof(sin);

    int reuse =1;
    socklen_t vallen=sizeof(reuse);
    if(setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,(const void*)&reuse,vallen)==-1)
    {
        perror("setsockopt error");
        return ;
    }
}

//释放资源
tftp_cli::~tftp_cli()
{
    //if(sockfd!=-1)
    close(sockfd);
}

 void tftp_cli::clearsceen()
{
    std::cout<<"按任意键继续\n";
    //getchar();  //吞掉上一个换行符  没事，已经被getline吞掉了

    //如果用户输入的是字符串呢，应该全部吞掉
    // if(getchar()!=-1) 
    // {
    //     // \033[2J : 清除整个屏幕
    //     // \033[H  : 将光标移动到左上角 (0,0)
    //     //std::cout << "\033[2J\033[H";
        
    //     system("clear"); // Linux/macOS 下清屏
    // }
    std::string temp="";
    std::getline(std::cin,temp);
    //if(temp!="")  getline会吞换行，但不会赋值。但实际上赋值不赋值不重要 
    system("clear");
}

void tftp_cli::showmenu()
{
    printf("---------------------------------\n");
    printf("-----欢迎来到tftp文件传输系统------\n");
    printf("---------------------------------\n");
    printf("请输出：1(download) 2(upload) 3(exit)\n");
}


//程序，方法执行的入口
void tftp_cli::run()
{
    
    std::string choice;
    while (1)
    {
        showmenu();
        //std::cin>>choice; 严谨点，空格也要读取
        std::getline(std::cin,choice);
        //int ch=stoi(choice);  但是这里有点问题，如果输入是字母字符，那么会报错
        //于是在这里检查异常试试
        int ch=4;
        try
        {
            ch=stoi(choice);
        }
        catch(const std::exception& e)
        {
            std::cerr <<"error fuction:"<< e.what() << '\n';
        }
        

        switch (ch)
        {
            case 1:
            DoDownLoad();
                break;
            case 2:
            DoUpLoad();
                break;
            case 3:
            std::cout<<"退出成功\n";
            if(sockfd!=-1) close(sockfd);
            exit(0);
                break;
            default:
            std::cout<<"输入错误，请重新输入\n";
                break;
        }
         clearsceen();
    }
    
}




