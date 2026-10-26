#include<iostream>
#include<unistd.h> //fork, getpid,ppid,_exit
#include<stdio.h>
#include<sys/types.h> //pid,ppid,wait,waitpid
#include<stdlib.h> //exit
#include<sys/wait.h> //wait,waitpid
using namespace std;
void test01() //区分父子进程
{
    int a=1;
    pid_t x=fork(); //返回值： 父进程返回子进程号，子进程返回0
    if(x!=0)
    {
        a++;
        cout<<"father\n";
        cout<<"a="<<a<<"\n";
    }
    else if(x==0)
    {
        cout<<"child\n";
        cout<<"a="<<a<<"\n";
    }
    else{
        perror("fork error!");
    }
}

void test02() //进程的数量为8不是3
{
    fork();
    fork();
    fork();
}

void test03()  //获取进程ID
{
     pid_t x=fork(); //返回值： 父进程返回子进程号，子进程返回0
    if(x!=0) //父进程
    {
        cout<<"myself="<<getpid()<<" father="<<getppid()<<"\n";
    }
    else if(x==0) //子进程
    {
        cout<<"myself="<<getpid()<<" father="<<getppid()<<"\n";
    }

    while(1);
}

void test04()   //进程退出,但是僵尸进程
{
     pid_t x=fork(); //返回值： 父进程返回子进程号，子进程返回0
    if(x!=0) //父进程
    {
        cout<<"父进程的id是"<<getpid()<<"\n";
        printf("你好呀");
    }
    else if(x==0) //子进程
    {
        cout<<"子进程的id是"<<getpid()<<"\n";
        printf("嗨嗨~");
        exit(1);  //刷新缓冲区，打印
        //_exit(1);  //不刷新缓冲区，不打印
    }

    while(1); //子进程退出，但父进程没有退出，子进程资源无法被回收,变成了僵尸态,
}

void test05()   //回收资源
{
     pid_t x=fork(); //返回值： 父进程返回子进程号，子进程返回0
    if(x!=0) //父进程
    {
       sleep(8);
       //x=wait(NULL);
       x=waitpid(x,NULL,WNOHANG); //非阻塞
       if(x==0) cout<<"回收失败\n";
       else cout<<"成功回收"<<x<<"\n";
       exit(1);
    }
    else if(x==0) //子进程
    {
       for(int i=1;i<=5;i++)
       {
            cout<<"你好呀\n";
            sleep(2);
       }
       exit(1); //显式退出
    }
}

//********其他案例**********

void test06()  //创建两个子进程并为其收尸
{
    // pid_t pid1=1,pid2=1;
    // pid1=fork();  //创建大儿子
    // if(pid1>1) //父进程
    // {
    //    pid2=fork(); //创建二儿子,同时它获得了父亲的数据pid1
    // }
    // if(pid2>1)
    // {
    //     cout<<"父进程\n";
    // }
    // else if(pid1==0)
    // {
    //     cout<<"大儿子\n";
    // }
    // else if(pid2==0)
    // {
    //     cout<<"二儿子\n";
    // }
    // exit(1);
    //太麻烦了
    pid_t pid1=fork(); //创建大儿子
    if(pid1>0) //父进程
    {
        pid_t pid2=fork(); //创建二儿子
        if(pid2>0)
        {
            sleep(3);
            cout<<"父进程\n";
            cout<<wait(NULL)<<"回收完毕\n";
            cout<<wait(NULL)<<"回收完毕\n";
        }
        else {
            sleep(4);
            cout<<"二儿子\n";
        }
    }
    else{
        sleep(1);
            cout<<"大儿子\n";
    }

}



void test07()  //孤儿进程
{
     pid_t x=fork(); //返回值： 父进程返回子进程号，子进程返回0
    if(x!=0) //父进程
    {
       sleep(3);
       exit(1);
    }
    else if(x==0) //子进程
    {
        while(1)
        {
            cout<<"你好\n";
            sleep(1);
        }
    }
}

int main(int argc,const char* argv[])
{
   test07();
}
