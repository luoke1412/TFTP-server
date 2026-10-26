//1.计算机在存储多字节整数时，主机默认小端存储，而网络字节序默认大端存储
//2.于是我们有两个接口
   //2.1 将4/2字节整数主机字节序(h)转换为网络字节序(network) hton(l/s)
   //2.2 将网络字节序转化为4/2字节整数主机字节序(host)  ntoh(l/s)    

//3.什么时候用： 网络传输用网络字节序， 接收端转化回来
#include<iostream>
#include<arpa/inet.h>
using namespace std;


void method01()
{
    int m=0x12345678;
    //四字节
    char* temp=(char*)&m;
    //一字节一字节的读取
    if(*temp==0x12)
    {
        cout<<"big endian\n";
    }
    else if(*temp==0x78)
    {
        cout<<"small endian\n";
    }
}

//法二： 定义共用体
void method02()
{
    union Info
    {
        int x;
        char head; 
    };
    union Info a;
    a.x=0x12345678;
    if(a.head==0x12)
    {
        cout<<"big endian\n";
    }
    else if(a.head==0x78)
    {
        cout<<"small endian\n";
    }
}

//网络字节序转化函数 
//注意：只提供了四字节和二字节的方法。
//单字符和字符串传输(单字节)不需要转化
void method03()
{
    uint32_t m=0x12345678;
    uint32_t temp=htonl(m);
    //将该数据转换为网络字节序
    printf("转化前为%#x,转化后为%#x\n",m,temp);
}

int main()
{
    //method02();
    method03();
}