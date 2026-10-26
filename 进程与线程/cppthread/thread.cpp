//演示C++线程创建，回收，获取线程号
#include<iostream>
#include<stdio.h>
#include<thread>
#include <chrono>
using namespace std;

//普通函数（如之前的 task1）：函数名在传参时会自动退化成函数指针
void task1()
{
    //this_thread::sleep_for(1s);
    cout<<"task1 sleep_for 1s\n";
    this_thread::sleep_for(chrono::seconds(1));
    
    cout<<"task1完成，tid="<<this_thread::get_id()<<"\n"; 
}

void task2(int num,char*p)
{
    cout<<"task2完成，tid="<<this_thread::get_id()<<"\n";
}

//类里面的函数.成员函数（如 &task3::print3）：C++语法规定，取类的非静态成员函数地址时，必须显式加 &
class task3
{

public:
    int a;  //this->a
    int b;  //this->b
    
    task3(int x,int y);
    //~task3();

    void print3(int a,int b);
};

task3::task3(int x,int y):a(x),b(y){}

void task3::print3(int a,int b){
         cout<<"task3完成，pid="<<this_thread::get_id()<<"\n";
}


//lambda表达式也可以


/*主函数本身就是主线程*/
int main(int argc, const char *argv[])
{
    thread th1;
    //构造函数
    thread th2(task1); //无参无返回值

    thread th3(task2,5,nullptr); //有参传参就行了   NULL被识别成了0

    task3 object(10,20);
    cout<<"th3.di="<<th3.get_id()<<endl; //主线程问我创建的th3的线程号是多少   this是自己问自己的


    //thread th4(&task3::print3,&object); //注:第一个参数是函数指针哦
    thread th4(&task3::print3,&object,10,20); //第二个默认有this指针，所以一定要传对象指针,
                                              //而你的函数本身就有a,b参数

    //lambda表达式 [](){}
    thread th5([](int a){
        cout<<"task5完成，pid="<<this_thread::get_id()<<"\n";
    },10);



    


    th2.join();
    th3.join();
    th4.join();
    th5.join();
    cout<<"hello world!\n";
    return 0;
}