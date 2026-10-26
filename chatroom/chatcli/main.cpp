//g++ chatcli.cpp main.cpp -pthread -std=c++11
// ./a.out  172.18.0.1 7777 lihua
//./a.out  172.18.0.1 8888 lisi
#include"chatcli.h"
using namespace std;
int main(int argc, const char *argv[])
{
     try
        {
           //业务代码
            if(argc<4)
            {
                std::cout<<"请输入ip,端口号和主机名\n";
                return 1;
            }
            const char* ip=argv[1];
            uint16_t port= (uint16_t)stoi(argv[2]);
            const char* name=argv[3];

            //初始化
            class chatcli client(ip,port);
            //跑起来
            client.run(name);
        }

        catch(const std::exception& e)
        {
            std::cerr <<"error fuction:"<< e.what() << '\n';
        }
    return 0;
}