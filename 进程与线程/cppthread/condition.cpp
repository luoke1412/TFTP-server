//演示C++中条件变量实现一个生产者和多个消费者的同步问题
/*
复习一下原理： 条件变量是内核空间的资源，具有原子性
他的原理是这个线程先抢到锁，如果满足执行条件就继续往下执行
如果不满足条件，就释放锁，进入阻塞队列。
知道有人来叫它，它才醒来抢锁。 检查是否满足执行条件，如果不满足继续睡觉

但是std::condition_variable是标准库用户态对象，
底层等待队列靠内核 futex/pthread 实现，不是“纯内核资源”
*/

//生产者：消费"空位"，生产"资源"
//消费者：消费"资源"，生产"空位"

#include<iostream>
#include<stdio.h>
#include<thread>
#include <chrono>
#include<vector>
#include<mutex>
#include<condition_variable>
using namespace std;
mutex m;
int source;  //资源
condition_variable cv;
//每生产一个资源，就叫醒一个线程
void producer()
{
    for(int i=1;i<=5;i++)
    {
        this_thread::sleep_for(chrono::milliseconds(1)); //每隔1ms生产一个
        //source是临界资源，需要保护
        {
            unique_lock<mutex>lock(m);
            source++;
            cout<<"生产资源：source="<<source<<"\n";
        }
       
        if(source>0)
        {
            cv.notify_all(); //叫醒所有生产者来抢
        }
        
    }
}


void consumer()
{
    //先抢锁，在释放锁之前都是原子操作
    unique_lock<mutex>lock(m); //unique_lock<mutex>是锁管理器对象，实现了RAII(初始化时得到锁，销毁时释放锁) 
    while(source<=0) 
    {
        cv.wait(lock);  //没有资源。传入管理锁对象释放锁，并阻塞     
    }
    source--;
    cout<<this_thread::get_id()<<"来消费资源了：source="<<source<<"\n";

}

/*主函数本身就是主线程*/
int main(int argc, const char *argv[])
{
    
    thread th1(producer);
    vector<thread> arr;

    for(int i=1;i<=5;i++)
    {
         //thread th2(consumer);
         //arr.emplace_back(th2);
         //1.直接在容器里造线程
         //arr.emplace_back(consumer); //存储是为了方便最后阻塞回收
        
         //2.用移动语句
         thread th2(consumer);
         arr.emplace_back(move(th2));
    }
   

    cout<<"hello world!\n";
    th1.join();
    for(auto&i :arr)
    {
        i.join();
    }
    
    
    return 0;
}