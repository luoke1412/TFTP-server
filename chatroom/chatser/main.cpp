//g++ chatser.cpp main.cpp -pthread -std=c++11
//./a.out 172.17.165.43 5555

#include"chatser.h"
using namespace std;
int main(int argc, const char *argv[])
{

    try
        {
           //业务代码

            //提示给出ip和端口号
            if(argc<3)
            {
                cout<<"请输入ip和端口号\n";
                return 1;
            }
            int size=(argc==4?stoi(argv[3]):5);  //默认是4



           //初始化
        class chatser server(size,argv[1],(uint16_t)stoi(argv[2]));

        server.run();
        }
        catch(const std::exception& e)
        {
            std::cerr <<"error fuction:"<< e.what() << '\n';
        }
    return 0;
}