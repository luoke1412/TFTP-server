/*牢记：点分十进制--->字符串，给我们看的
        接口：inet_ntoa  网络到ASCII码
        四字节无符号整数-->十六进制地址，网络字节序列，网络传输用的
        接口：inet_addr 地址  
*/
// network:网络号   address:地址  to:转换 
#include<iostream>
#include<arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

//点分十进制转化为四字节无符号网络字节序(十六进制地址)
void test01()
{
    const char* ip="192.168.24.8";

    uint ip_net=inet_addr(ip);
    printf("%#x",ip_net);
}

//将4字节无符号整数的网络字节序，转换为点分十进制的字符串

void test02()
{
    //int_addr ip_net=0x818a8c0;
   // 这是在一个结构体里面。并且已经定义好了的
    struct in_addr addr;
    //(2)必须转化为网络字节序
    addr.s_addr=htonl(0x12345678);


    const char* ip=inet_ntoa(addr);
    printf("%s",ip);
}

using namespace std;
int main()
{
    //test01();
    test02();
}