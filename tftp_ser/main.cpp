#include"tftp_ser.h"
using namespace std;
int main(int argc, const char *argv[])
{
    //初始化一个服务器对象
    class tftp_ser server;
    //运行服务器
    server.run();
    
    return 0;
}