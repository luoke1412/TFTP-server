#include"threadpool.h"

//私有方法
 //启动和初始化线程池(具体来讲，创建n个任务线程等待任务)
void threadpool::runthreadpool(int tsize)
{
    //创建n个任务线程
    for(int i=1;i<=tsize;i++)
    {
        //v.emplace_back(); 按经验，这里传入函数，就地创造线程
        //用lambda表达式
        v.emplace_back([this](){ //this:当前工作线程对象  
            /*函数体内容：多线程竞争任务队列资源-->条件变量+互斥锁*/
            
           //因为是线程复用，所以套一层循环
            while(1)
            {
                std::function<void()>f; //定义存储任务函数对象
                {
                    std::unique_lock<std::mutex>lock(m);

                    //如果没有任务并且没停  就睡觉,并且释放锁   注：判断stop也很重要
                    //while是为了防止线程 自己醒来 往下执行的情况
                    #if 0
                    while(q.empty()&&!stop) 
                    {
                        cv.wait(lock);
                    }  
                    #endif   //这是写法一，写法二：
                   
                    cv.wait(lock,[this](){ //0:阻塞 1：执行，其底层就是封装了while循环
                        if(q.empty()&&!stop) return false;
                        else return true;
                    });

                    //至此，线程已经拿到了锁，但是有两种情况
                    //一是线程池准备退出，把所有的线程全部唤醒。如果任务队列里确实没任务了，就返回
                    //二是有任务，继续执行任务
                    if(q.empty()&&stop==true)  
                    {
                        std::cout<<std::this_thread::get_id()<<"正常退出\n";
                        return;
                    }

                    //拿出任务来执行
                    f=q.front();
                    q.pop();
                    std::cout<<"删除成功，q.size()="<<q.size()<<"\n";
                     std::cout<<std::this_thread::get_id()<<"执行了任务"<<"\n";
                }//至此可以释放锁了，后面就是线程执行具体任务的过程，不需要被保护
                
                f(); //执行
            }

        });
    }
} 

//添加任务 (删除由线程函数体逻辑执行)
#if 0
void threadpool::addtask(void(*f)(void))  函数指针版
#endif  
void threadpool::addtask(std::function<void()>f)
{
    //要修改任务队列，加锁保护
    {
        std::unique_lock<std::mutex>lock(m);
        q.push(f);
        std::cout<<"添加成功，q.size()="<<q.size()<<"\n";
    }
    //因为添加了一个，叫醒一个线程
    cv.notify_one();
}


//共有方法
//定义时初始化
threadpool::threadpool(int n){
    runthreadpool(n);

}
//被销毁时 回收所有线程资源
threadpool::~threadpool(){
    {
        std::unique_lock<std::mutex>lock(m);
        stop=true;  //因为stop也被其他工作线程共享，所以希望这个操作是原子性的
    }//销毁时自动释放锁
    
    //唤醒正在睡觉的线程  但是！有一种可能，就是你在执行这句话的时候线程还没有睡觉，
    //但是执行完的时候线程才执行到发现为空，睡觉 ...其实改一改上面的就好，顺序问题
    cv.notify_all();
    //阻塞回收所有线程资源
    for(auto&i:v)
    {
        i.join();
    }

}