#include "tftp_cli.h"
using namespace std;
int main(int argc, char const *argv[])
{
    short cli_port=8888;
    if(argc>1)
    {
        cli_port=(short)atoi(argv[1]); //字符转整型
    }
    //初始化
    tftp_cli client(cli_port);
    //启动！
    client.run();

    return 0;
}
