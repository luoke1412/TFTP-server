//测试：将容器的内容写入到外部文件中，然后从外部文件读取数据到容器，容器写入内容到终端
//open本质：路径名转换为文件描述符
//write -在一个文件描述符上执行写操作
//read - 在文件描述符上执行读操作
#include<iostream>
#include<string.h>
 #include <unistd.h>
#include<stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
using namespace std;
int main(int argc, const char *argv[])
{
    //(1)打开文件
    int fd=-1;
    fd=open("./open",O_WRONLY|O_CREAT,0644);
   // fd=open("./open",O_WRONLY);  不存在则报错
    if(fd==-1)
    {
        perror("open error");
    }

    //(2)往文件里写数据之前，先准备容器
    char wbuf[128];
    fgets(wbuf,20,stdin);  //从终端输入
    wbuf[strlen(wbuf)-1]='\0'; //最后一个换行符改为为反斜杠0 

    //(3)写
    write(fd,wbuf,strlen(wbuf));
    
    close(fd);

    /********************************/

    //(4)关闭后重新以只读的方式打开
    fd=open("./open",O_RDONLY);
   // fd=open("./open",O_WRONLY);  不存在则报错
    if(fd==-1)
    {
        perror("open error");
        return -1;
    }

    //(5)定义容器接收外部文件的数据
    char rbuf[128];
    ssize_t res=read(fd,rbuf,30); //成功的话返回读到的字节数
    if(res==-1)
    {
        perror("read error");
        return -1;
    }
    cout<<"读到的字节数是"<<res<<"\n";
    fprintf(stdout,"内容是：");
    fputs(rbuf,stdout);
    return 0;
}