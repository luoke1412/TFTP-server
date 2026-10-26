//消息队列：演示发送数据，接收方能收到
//并且当发送quit，双方都结束通信。最后要有一方把消息队列给销毁
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
  //2.根据key值，打开(已存在)消息队列，并返回它的ID
    //int qid=msgget(key,IPC_CREATE|0644);
   int msgqid=msgget(key,IPC_CREAT|0664);  //如果是新创建的，就加上权限位
   if(msgqid==-1)
   {
     perror("msgget error");
   }
   //cout<<msgqid<<"\n";

   while(1)
   {
      //sleep(1);
      msgBuf msg;  //用于接收消息的容器
     // msgrcv(msgqid,&msg,MSGSZ,0,0);  //接收消息的类型和是否阻塞

    //只接受消息类型为1的，并且阻塞接收  
    ssize_t n = msgrcv(msgqid, &msg, MSGSZ, 1, 0);
    if (n == -1) {
      perror("msgrcv failed");
      exit(1);
    }
      
      cout<<"消息类型是"<<msg.mtype<<"\n";
      cout<<"正文大小是"<<msg.mtext<<"\n";
      //if(msg.mtext=="quit")
      if(strcmp(msg.mtext,"quit")==0)
      break;
   }

   //销毁消息队列
   if(msgctl(msgqid,IPC_RMID,NULL)==-1)
   {
     perror("msgctl error");
     return -1;
   }
   printf("退出成功\n");
   return 0;
}