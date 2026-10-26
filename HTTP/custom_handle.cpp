#include "custom_handle.h"
#include "http.h"
#define KB 1024
#define HTML_SIZE 64*KB
int handle_add(int sockfd,char* req_buf)
{
    /*
    1.把容器里面的数据解析出来
    2.执行加法
    3.转成字符串发送过去
    */

    int num1,num2;  //先说明一下，这里用int存储，重在演示效果
                    //更安全的是高精度加法
    int ret=sscanf(req_buf,"\"data1=%ddata2=%d\"",&num1,&num2);
    //从字符串中读取格式化数据
    if(ret==-1)
    {
        perror("sscanf error");
        return -1;
    }

    char rep_buf[HTML_SIZE];
    //sprintf(rep_buf,)
    sprintf(rep_buf,"%d",num1+num2);  //将int转化为字符串存储在容器中
    printf("rep_buf=%s",rep_buf);

    send(sockfd,rep_buf,strlen(rep_buf),0);
    return 0;
}


void handle_login(int sockfd,char* req_buf)
{
    /*
    1.获取用户输入的账号和密码
    2.与后台数据库匹配，看是否符合
    3.登录成功，发送正文： 跳转到主页
    4.如果登录失败...
    */

    //username=admin&password=123
    char*uname=NULL;
    char *pwd=NULL;
    uname=&req_buf[0];
    uname+=strlen("username=");
    pwd=uname;
    while(*pwd!='=')
    {
        pwd++;
        if(*pwd=='&') *pwd=0;
    }    
    pwd++;
    printf("username=%s  password=%s\n",uname,pwd);

    //与数据库进行匹配，但是这里没有，
    //所以登录成功的逻辑暂定为用户名==密码
    if(strcmp(uname,pwd)==0)
    {
        printf("login success!\n");
        //发送正文??? 要发送啥？前端要啥
        char rep_buf[HTML_SIZE];

        // //程序执行至此，表示登录成功，此时可以跳转到首页
        // sprintf(rep_buf, "<script>localStorage.setItem('usr_user_name', '%s');</script>", uname); //用户名


        // strcat(rep_buf, "<script>window.location.href='/index.html';</script>"); //要跳转的页面
        //...
        int fd=open("root/index.html",O_RDONLY);
        struct stat st;
        stat("root/index.html",&st);
        sendfile(sockfd,fd,NULL,st.st_size);

        //消息已经组装成功，此时发给网页端

        //send(sockfd, rep_buf, strlen(rep_buf), 0);
    }
}


//gpt.html 发来的请求：content=任意内容
//业务逻辑：无论内容是什么，都让浏览器跳到 /marry.html
//做法跟 handle_login 一致，不靠 302，直接回一段 JS 让前端跳转（响应仍是 200）
void handle_gpt(int sockfd,char* req_buf)
{
    /*
    1.识别出这是 gpt.html 发来的请求（前端带了 gpt=1 标记）
    2.不关心 content 具体是什么
    3.回一段 <script>，把浏览器重定向到 /marry.html
    */

    char rep_buf[HTML_SIZE];
    //组装跳转脚本：先存一个标记（可选，留着好排查），再跳转
    sprintf(rep_buf,"<script>localStorage.setItem('gpt_from_gpt', '1');</script>");
    strcat(rep_buf,"<script>window.location.href='/marry.html';</script>");

    printf("gpt request captured, redirect to /marry.html\n");
    send(sockfd,rep_buf,strlen(rep_buf),0);
}


//定义处理业务相关的函数
void parse_and_proccess(int sockfd,char* req_buf,char* request_string)
{
    //这里统一先发送响应报文的固定内容
    const char*msg="http/1.1 200 OK \r\n\r\n";
    send(sockfd,msg,strlen(msg),0);

    //post请求
    if(request_string==NULL)
    {
         //前端发来的数据："data1=1data2=2"   执行加法运算
         
         if(strstr(req_buf,"data1=")!=NULL&&strstr(req_buf,"data2=")!=NULL) 
         //finds the first occurrence of the substring needle in the string haystack
         //return a pointer to the beginning of the located substring, 
         //or NULL if the  substring  is  notfound
        {
             int ret=handle_add(sockfd,req_buf);
             if(ret==-1)
             {
                printf("req_add error!\n");
             }
        }

        //发来的数据：username=admin&password=123
        //则执行登录操作
         else if(strstr(req_buf,"username=")&&strstr(req_buf,"password="))
         {
            handle_login(sockfd,req_buf);
         }

        //gpt.html 发来的请求：带 gpt= 标记
        //无论 content 是什么，都跳转 marry.html
         else if(strstr(req_buf,"gpt=")!=NULL)
         {
            handle_gpt(sockfd,req_buf);
         }

         else{
            //其他请求....
         }
    }
   //带数据的get请求
    else
    {

    }
}
