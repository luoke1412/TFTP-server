//无名管道通信
#include<iostream>
#include<unistd.h> //fork, getpid,ppid,_exit,write,read,open
#include<stdio.h>
#include<sys/types.h> //pid,ppid,wait,waitpid
#include<stdlib.h> //exit
#include<sys/wait.h> //wait,waitpid
#include <string.h>
using namespace std;


int test01() //验证父亲写数据，孩子读数据
{
    //可以在此创建管道文件，并返回该管道文件的两端，那么父子进程都会拥有该管道两端的文件描述符
    int fildes[2];  //存放管道文件的两端文件描述符   0:读  1:写

    if(pipe(fildes)==-1)
    {
        perror("pipe error\n");
        //exit(1)
        return -1;
    }
    else 
    {
        //printf("%d %d\n",fildes[0],fildes[1]);   3,4
    }

    
    pid_t pid=fork();
    //开始进行读写操作
    if(pid>0) //父亲·
    {
        //如果不读，可以关掉
        close(fildes[0]);

        char wbuf[100]="hello";   //Write Buffer
        //将上述数据发送给子进程,只需将数据通过写端写入管道文件中
        write(fildes[1],wbuf,strlen(wbuf));

        close(fildes[1]);
        wait(NULL); //等待回收子进程
    }
    else if(pid==0) //孩子
    {
        char rbuf[10]="";
        //如果不写，可以关掉
        close(fildes[1]);

        read(fildes[0],rbuf,sizeof(rbuf));

        cout<<rbuf<<"\n";

    }
    else
    {
        perror("fork error!\n");
        return -1;
    }
    return 0;
}

int test02() //验证管道有限,64KB
{
    int fildes[2];
    if(pipe(fildes)==-1)
    {
        perror("pipe error!\n");
        return -1;
    }

    pid_t pid=fork();
    if(pid>0)
    {
        //父亲
        int cnt=0;
        char x='A';
        while(1)
        {
            write(fildes[1],&x,sizeof(x)); 
            cnt++; 
            cout<<cnt<<"\n";  //65536
        }
    }
    else if(pid==0)
    {
        //孩子
    }
    else{
        perror("fork error!\n");
        return -1;
    }
}

int test03() //验证写操作没了，读完了之后不会阻塞
{
     
    int fildes[2];  //存放管道文件的两端文件描述符   0:读  1:写

    if(pipe(fildes)==-1)
    {
        perror("pipe error\n");
        //exit(1)
        return -1;
    }
    pid_t pid=fork();
    //开始进行读写操作
    if(pid>0) //父亲·
    {
       
        char wbuf[100]="hello";   //Write Buffer
        //将上述数据发送给子进程,只需将数据通过写端写入管道文件中
        write(fildes[1],wbuf,strlen(wbuf));
         close(fildes[1]);
        
        wait(NULL); //等待回收子进程
    }
    else if(pid==0) //孩子
    {
        char rbuf[10]="";
        //如果不写，可以关掉
        close(fildes[1]);

        read(fildes[0],rbuf,sizeof(rbuf));

        cout<<rbuf<<"\n";

    }
    else
    {
        perror("fork error!\n");
        return -1;
    }
    return 0;
}


int test04()  //验证读操作没了，写操作会炸
{
    int fildes[2];  //存放管道文件的两端文件描述符   0:读  1:写

    if(pipe(fildes)==-1)
    {
        perror("pipe error\n");
        //exit(1)
        return -1;
    }
    pid_t pid=fork();
    //开始进行读写操作
    if(pid>0) //父亲·
    {
       
        char wbuf[100]="hello";   //Write Buffer
        //将上述数据发送给子进程,只需将数据通过写端写入管道文件中
        while(1)
        {
            write(fildes[1],wbuf,strlen(wbuf));
            close(fildes[0]);
        }
        
    }
    else if(pid==0) //孩子
    {
         close(fildes[0]);
       // read(fildes[0],rbuf,sizeof(rbuf));


    }
    else
    {
        perror("fork error!\n");
        return -1;
    }
    return 0;
}
int main(int argc,const char*argv[])
{
   // test01();
    test04();
}
