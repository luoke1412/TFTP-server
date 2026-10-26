//msgtyp 参数是"筛选器"，不是"你要接收什么字段"。所有字段都会被接收回来。
//消息队列：演示发送数据，接收方能收到
//并且当发送quit，双方都结束通信。最后要有一方把消息队列给销毁

/*应用：演示接收方只能接收消息类型为1的消息*/
#include<iostream>
#include<stdio.h>
#include<string.h>
#include"/home/tyy/cppcode/head.h"
using namespace std;

//消息类型的定义
struct msgBuf{
long mtype; // 消息的类型
char mtext[1024]; //消息正文
};

#define MSGSZ (sizeof(msgBuf)-sizeof(long))

int main(int argc, const char *argv[])
{
  //1.创建返回key值
   key_t key=ftok("key",'k');
  //2.根据key值，创建消息队列，并返回它的ID
   //int msgqid=msgget(key,IPC_CREAT|IPC_EXCL|0644); //确保创建的是新的消息队列
   int msgqid=msgget(key,IPC_CREAT|0644);  //如果已存在就打开，不存在才创建并给出权限
   if(msgqid==-1)
   {
     perror("msgget error");
     return -1;
   }
   //cout<<msgqid<<"\n";

   //3.向消息队列中发送消息
   //首先要要构造消息
   while(1)
   {
        //构造消息
        msgBuf msg;
        printf("请输入消息类型:");
        scanf("%ld",&msg.mtype);
        getchar(); //因为马上要输入字符串了，换行符还在缓冲区里，把换行符给吃掉
        printf("请输入消息正文");
        fgets(msg.mtext,MSGSZ,stdin);
        msg.mtext[strlen(msg.mtext)-1]='\0'; //别让接收方把换行也给接受了

        //把消息正文放入到消息队列中
        msgsnd(msgqid,&msg,MSGSZ,0);
        if(strcmp(msg.mtext,"quit")==0)
         break;
   }
   //接收端销毁消息队列
  return 0;
}
