#pragma once
#include<iostream>
#include<thread>
#include<condition_variable>
#include<mutex>
#include<functional>
#include<queue>

//定义线程池类
class threadpool
{
    private:
    //std::function 是一个模板类，可以存储、调用和复制任何可调用对象，比如函数、lambda 表达式或函数对象。
    std::queue<std::function<void()>>q; //任务队列，类型是函数？
    std::vector<std::thread>v; //工作线程容器
    std::mutex m;  //锁变量，被抢的对象
    std::condition_variable cv; //条件变量  实现多线程同步
    bool stop=false;  //标志位：是否停止。默认不停止

    //私有方法
    void runthreadpool(int tsize);  //启动和初始化线程池
    
    //共有方法
    public:

    //函数指针
    #if 0
     void addtask(void(*func)(void));  //添加任务 (删除由线程函数体逻辑执行)
    #endif
    void addtask(std::function<void()>fun);
   

    threadpool(int n);
    ~threadpool();
};
