#include "../../myhead.h"
//UDP里面，客户端要向服务器发送消息，那它肯定需要知道服务器的套接字地址
//客户端发消息的同时也把它的地址信息给了服务器  
//#define CLI_PORT 8888
#define CLI_IP "172.17.165.43"
#define SER_IP "172.18.0.1"
#define SER_PORT 69  //这个是默认吧
#define MSG_SIZE 516



//客户端类
class tftp_cli  
{
    private:
    //const short port=CLI_PORT;
    short port;
    const char* ip=CLI_IP;  //要写死吗？待考虑。  其实可以用户输入的，那么固定IP让用户输入端口号吧
     
    int sockfd;
    /***********以上肯定得要的,其他东西用到再补*******/

    sockaddr_in sin;  //服务器的套接字地址信息
    socklen_t slen; 
    std::string filename;  //要上传/下载的文件名
    char msg[MSG_SIZE];  //存储报文


    //两个主方法
    void DoDownLoad();

    void DoUpLoad();

    //其他辅助方法(不想单独写个类了哈哈)
    void showmenu();

    void clearsceen();


    //共有方法
    public:
    //初始化
    tftp_cli(short port);
    //释放资源
    ~tftp_cli();
    //程序，方法执行的入口
    void run();
};