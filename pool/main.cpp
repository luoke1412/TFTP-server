#include<iostream>
#include<stdio.h>
#include"threadpool.h"
using namespace std;


void task() {
    cout<<"\n"; //任务内容不重要
}


int main(int argc, const char *argv[])
{
    //定义并初始化了三个任务线程
    threadpool thp(3);
    //创建任务
    for(int i=1;i<=20;i++)
    {
        //thp.addtask(task);

        //也可以写lambda表达式
        thp.addtask([](){
            cout<<"\n";
        });
    }
    return 0;
}

