#include "http.h"
#include"custom_handle.h"
//初始化服务器
int Init_Http()
{
    //std::cout<<HTTP_IP;

    //1.创建套接字(信息的载体)并返回它的文件描述符
    int sfd=-1;
    if((sfd=socket(AF_INET,SOCK_STREAM,0))==-1)  
    //地址族因特网(ipv4), 流式(tcp), 默认协议
    {
        perror("socket error");
        return -1;
    }


    printf("socket create success!sfd=%d\n",sfd);

     int reuse =1;
    socklen_t vallen=sizeof(reuse);
    if(setsockopt(sfd,SOL_SOCKET,SO_REUSEADDR,(const void*)&reuse,vallen)==-1)
    {
        perror("setsockopt error");
        return -1;
    }

    //2.绑定-将IP地址和端口号与套接字绑定，以使别人能找到我

        //2.1 填充要绑定的ip地址,端口号,地址族结构体 。  _in是ipv4专属
    struct sockaddr_in sin;
    sin.sin_family=AF_INET;
    sin.sin_addr.s_addr=inet_addr(HTTP_IP);  //wtm结构体里面是结构体来着

    sin.sin_port=htons(HTTP_PORT); //传输16位整数嘛

        //2.2填嘛，细节太多了
    if(bind(sfd,(const sockaddr*)&sin,sizeof(sin))==-1)
    {
        perror("bind error");
        return -1;
    }
     std::cout<<"绑定成功\n";

    //3.套接字启动监听模式(监听)
    if(listen(sfd,128)==-1)
    // 128:已连接队列的最大长度
    //也就是说，最多允许有 128 个已经连好的客户端在后台排队等 accept()
    {
        perror("listen error");
        return -1;
    }
    printf("listen success!\n");

    return sfd; 
}


//获取整行数据
//同时把 \r\n（Windows/网络协议风格）或单独的 \r（旧Mac风格）统一转换为 \n（Unix/Linux风格）存入缓冲区
int get_line(char buf[],int sockfd)  //引用，为了修改buf指针
{

    int k=0; //buf的下标
    //char temp[SIZE];  
    //buf=temp;  好家伙，栈里面定义的空间，返回时就会被回收，于是buf变成悬空指针了
    //策略：不要赋值指针，靠值传递


    char t;
    while(k<SIZE)
    {
        //一个字符一个字符的读取并判断
        if(recv(sockfd,&t,1,0)==-1)
        {
            perror("recv erorr");
            continue;
        }

        //判断t是不是\r   把\r\n \r都转化为\n，并且整行读完
        if(t=='\r')
        {
            if(recv(sockfd,&t,1,MSG_PEEK)==-1)
            {
                perror("recv erorr");
                continue;
            }

            if(t=='\n')  //是\n，把换行读出来，此时进入了第二行
            {
                recv(sockfd,&t,1,0);
            }

            else  //说明已经在第二行了
            {
                t='\n';
            }
        }

        buf[k++]=t;
        if(t=='\n') break;
    }
    buf[k]=0;
    //strcpy(buf,temp);  //值拷贝
    return k;
}

//定义清空头部的函数:
void clear_header(int sockfd)
{
    char buf[SIZE];
    //剩余部分： 若干请求首部+换行+实体主体(暂时不考虑)

    /*  这个代码直接炸了好吗，如果缓冲区中没有任何数据，recv是会阻塞的，所以并不会执行到ret==0
    while(1)
    {
        bzero(buf,sizeof(buf));
        int ret=recv(sockfd,buf,SIZE-1,0);   
        if(ret==0)
        {
            printf("剩余头清除完毕!\n");
            break;
        }
        else if(ret==-1)
        {
            perror("recv erorr");
            return -1;
        }
    }
    */

    //那么就固定一行一行的读(反正写了函数) 结束条件是读到最后的换行
    while(1)
    {
        bzero(buf,sizeof(buf));
        int ret=get_line(buf,sockfd);
        buf[ret]=0;
        if(ret==1&&strcmp(buf,"\n")==0) //你把它都转化为了\n
        {
            printf("成功清理消息头!\n");
            break;
        }
    }

    return;
}

int show_404(int sockfd) //展示"文件不存在"的html文件
{
    //先打开html文件
    int fd=open("root/404.html",O_RDONLY);
    if(fd==-1)
    {
        perror("open error");
        return -1;
    }
    //打开了文件,然后怎么发送给客户端呢？
    //嘿，先封装成响应报文的格式，再发送过去
    const char*msg="http/1.1 404 Not Found \r\n";  //字符串是常量，其首地址是常指针
    
    if(send(sockfd,msg,strlen(msg),0)==-1)
    {
        perror("send erorr");
        return -1;
    }
    send(sockfd,"\r\n",strlen("\r\n"),0);

    //接下来发送html文件
    //send(sockfd,fd,)  ohno，send只能发送缓冲区(字符数组)中的数据，不能发送文件本身
    //                            而且文件只有文件描述符标识它    
        //将整个文件发送出去
        //函数原型：ssize_t sendfile(int out_fd, int in_fd, off_t *offset, size_tcount);
        //功能：将一个文件描述符中的数据发送到另一个文件描述符中
        //参数1：要被拷贝到的文件描述符
        //参数2：从哪个文件中拷贝
        //参数3：从该文件的哪个位置开始拷贝
        //参数4：拷贝的文件大小
    //我需要文件的属性
    struct stat st;
    if(stat("root/404.html",&st)==-1)
    {
        perror("stat erorr");
        return -1;
    }
    //If offset is NULL, then data will be read from in_fd starting at the file offset
    if(sendfile(sockfd,fd,NULL,st.st_size)==-1)
    {
        perror("sendfile erorr");
        return -1;
    }
    close(fd);
    return 0;
}

void echo_error(int err_no,int sockfd) //错误码---根据不同的错误码打印出不同的提示页面
{
    switch (err_no)
    {
    case 404:
        show_404(sockfd);
        break;
    case 405:
        //show_405();
        break;
    
    case 500:
        //show_500();
        break;
    default:
        break;
    }
}


int echo_www(int sockfd,char*path)
{

    if(strcmp(path,"./root/")==0) //说明用户没给指定路径，拼接默认路径 
    {
        //snprintf(path,SIZE,"./root/index.html");
        strcat(path,"index.html");
    }

    printf("newpath=%s\n",path);
    //但是有个问题，搜索框显示的不是newpath嘞


    struct stat st;
    stat(path,&st);

    //先打开html文件
    int fd=open(path,O_RDONLY);
    if(fd==-1)
    {
        perror("open error");
        return -1;
    }

    //然后封装成响应报文的格式依次发送
    const char*buf="http/1.1 200 OK \r\n\r\n";
    send(sockfd,buf,strlen(buf),0);

    //发送html文件
    sendfile(sockfd,fd,NULL,st.st_size);
    return 0;
}

void handle_request(int sockfd,char *method,char *request_string)
{
    //如果是post，我要获取它的请求数据(首部字段有长度，数据正文在实体主体)
    //如果是get，因为已经存储在了string,可以直接清空掉头部
    int req_len;
    char req_buf[SIZE]; //存储请求数据
    char buf[SIZE];
    if(strcmp(method,"GET")==0)
    {
        clear_header(sockfd);
    }
    else
    {
       // int cnt=0;
        while(1) //从首部开始读取
        {
            bzero(buf,sizeof(buf));
            int ret=get_line(buf,sockfd);  //一行一行的读，并且把\r\n换成了\n
            if(ret==1&&strcmp(buf,"\n")==0)  //出口是读到最后的换行符
            {
                //printf("cnt=%d",cnt);
                break; 
            }

            //读出正文长度字段的值
            if(strncasecmp(buf,"content-length",14)==0)
            {
                req_len=atoi(buf+strlen("content-length")+2); //+2因为还有冒号，空格
            }
        }
        //获取正文数据
        recv(sockfd,req_buf,req_len,0);
        req_buf[req_len]=0;
    }
    //输出相关的数据
    printf("req_len=%d,req_buf=%s\n",req_len,req_buf);

    //请求数据都已经获取了，那么接下来就是执行对应的业务逻辑并发送响应报文
    parse_and_proccess(sockfd,req_buf,request_string);
}


//处理消息请求的入口
/*  接收+解析步骤如下：
1. 获取请求行：调用 get_line，把 \r\n统一换成 \n。
2. 截取请求方法：从请求行中提取出 GET或 POST。
3. 截取 URL 和查询参数：如果遇到 ?，把 ?前面的路径存入 url数组，把 ?后面的参数存入 query_string指针。
4. 修饰路径：把提取出的 url（比如 /index.html）拼接到服务器本地的网站根目录 wwwroot下，
形成真实物理路径 path（比如 wwwroot/index.html）。为什么截取完 URL 才去修改路径？ 
因为你从请求行里提取出来的是相对路径（如 /index.html）。服务器不能直接去硬盘根目录找 /index.html，
必须把它“翻译”成服务器程序自己目录下的绝对路径（如 wwwroot/index.html），这样后续才能正确打开文件。
5. 判断文件是否存在：使用 stat函数检查这个物理路径是否存在。
*/
int handle_msg(int sockfd)
{
    char del_buf[SIZE];  

    //读取客户端发来的请求
    //通常我们使用recv接收客户端请求时，最后一个参数如果是0表示阻塞读取，并将数据读走
    //而最后一个参数为MSG_PEEK时，仅仅指示表示查看套接字中的数据，并不读走
    //相当于拷贝了一份数据
    recv(sockfd,del_buf,sizeof(del_buf),MSG_PEEK);
    
    #if 1 //看一看内容
    printf("-------------------\n");
    printf("%s",del_buf);
    printf("-------------------\n");
    #endif

    //1.真正开始读取一行数据
    char buf[SIZE]=" ";
    int ret=get_line(buf,sockfd);  //好家伙，直接传首地址修改地址存储的值
    
    printf("buf=%s\n",buf);

 /*   不能一次性全清理了，如果是post你要先找到它的请求数据才行嘞   
    //1.5 清除消息头部(因为上面解析数据只读取了其中一行)
    //目的：如果是持续连接必须刷新接收缓冲区，避免产生影响。  
    //但也要知道其他情况：(1)短连接影响不大(2)不影响其他客户端--因为每次都要创建新的通信套接字
    clear_header(sockfd);
*/

    //2.解析 请求方法，URL及查询参数
    char method[20];
    int i=0;  //保证接着用
    for(i=0;i<ret;i++)
    {
        if(buf[i]==' ') break;
        method[i]=buf[i];
    }
    method[i]=0;

    printf("method=%s\n",method);
        //跳过所有空格
    while(buf[i]==' '&&i<ret) i++;
 
    char url[ret];
    int k=0;
    char* request_string=NULL;  //请求参数的首地址，默认NULL，表示没有

    for(;i<ret;i++)
    {
        if(buf[i]==' ')
        {
            break;
        }
        else if(buf[i]=='?') //说明有请求参数，需要处理请求数据
        {
            request_string=&buf[i];
            request_string++;

            while(buf[i]!=' ') i++;  //循环，遇到第一个空格时改成结束符
            
            buf[i++]=0;
            break;
        }
        else  //普通的URL直接放入数组
        {
            url[k++]=buf[i];
        }
    }
    url[k]=0;

    printf("url=%s\n",url);
    if(request_string!=NULL) printf("request_string=%s\n",request_string);
    //3.获取的url是服务器的相对地址，把请求资源路径固定为 wwwroot 下的资源
    //也就是我实际的地址
    //strcat(url,"wwwroot");  这个是添加到末尾把
    char path[SIZE];
    sprintf(path,"./root%s",url);   //url本身包含反斜杠

    printf("path=%s\n",path);

    //至此，请求行已全部解析出来了，用数组存储，
    //4.开始分别执行不同的响应
    
    //情况1：出错，返回对应的界面

        //请求方法出错--->405
    if(strcasecmp(method,"POST")!=0&&strcasecmp(method,"GET")!=0)
    {
         //清空消息头
        clear_header(sockfd);
        printf("method erorr\n");
        echo_error(405,sockfd) ;
    }

        //请求的文件在该路径下不存在--->404
    //那么怎么判断文件是否存在呢？  与文件状态相关的结构体和函数
    struct stat st;
    if(stat(path,&st)==-1) // 如果路径存在的话，会把文件信息赋值给st
    {
         //清空消息头
        clear_header(sockfd);
        printf("stat erorr!\n");
        echo_error(404,sockfd);
        return -1;
    } 

/************************************************ */
    //情况2  普通的get请求，直接返回页面
    else if(strcmp(method,"GET")==0&&request_string==NULL)
    {
        /*
        如果用户输入的是http://172.17.165.43:8890/index.html，返回index.html
        这里还要考虑到一种情况，就是如果用户他没有输入路径，eg 1.0.0.0 默认是根目录 1.0.0.0 /
        此时应该返回默认路径对应的页面。  上面被拼接成了./root/了吗 运行看看，是的 
        同时用户也可以自己写清楚路径，访问具体哪个页面(前提是有)
        */

         //清空消息头
        clear_header(sockfd);  
        if(echo_www(sockfd,path)==-1)
        {
            printf("echo_www erorr!\n");
            return -1;
        }
    }

    //情况3           带数据的get请求   以及    post请求
//地址栏输入 172.20.10.2:8080/add?a=1&b=2   前端 JS 触发 xhr.send("a=1&b=2")
    else if((strcmp(method,"GET")==0&&request_string!=NULL)||
            strcmp(method,"POST")==0)
    {
        //处理请求数据
        handle_request(sockfd,method,request_string);
    }

    //全部结束时，关闭客户端
    close(sockfd);
    //pthread_exit(NULL); //退出线程
    return 1;
}

