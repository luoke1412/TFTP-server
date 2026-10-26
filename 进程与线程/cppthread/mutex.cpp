//演示C++中使用锁保护临界区
#include<iostream>
#include<stdio.h>
#include<thread>
#include <chrono>
#include<mutex>
using namespace std;
mutex m;

//需求：线程执行完它的任务后再让其他线程执行
void task1()
{
   // unique_lock<mutex>是锁管理器对象，实现了RAII(初始化时得到锁，销毁时释放锁) 
    unique_lock<mutex> lock(m); //获取锁资源，保护接下来的代码块。
                                //当生命周期到了，调用析构函数会自动解锁

    cout<<"task1 sleep_for 1s\n";
    this_thread::sleep_for(chrono::seconds(1));
    //this_thread::sleep_for(1s);
    cout<<"task1完成，tid="<<this_thread::get_id()<<"\n";

}


/*主函数本身就是主线程*/
int main(int argc, const char *argv[])
{
    thread th1;
    //构造函数
    thread th2(task1); //无参无返回值

    this_thread::sleep_for(chrono::milliseconds(500));
    cout<<"hello world!\n";
    th2.join();
    
    return 0;
}