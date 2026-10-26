/*练习：使用多进程完成两个文件的拷贝工作，父进程拷贝前一半内容，子进程拷贝后一半内容，父进程
要回收子进程的资源
拆分步骤
0.这次用到外部文件传参  argc:表示文件个数  argv: 是一个指针数组，argv[0]就表示字符串文件名a.out
先检查文件个数是不是3个
1.创建子进程
2.创建目标文件,打开源文件，获得源文件的长度，为拷贝做准备
3.父子进程分别拷贝前一半和后一半内容
4.父进程等待回收子进程资源
*/
#include<iostream>
#include<stdio.h>
#include<cmath>
#include "../head.h"
using namespace std;

int get_len(const char* srcfilename,const char* destfilename) //创建目标文件,打开源文件，获得源文件的长度，为拷贝做准备
{
    int srcfd=-1,destfd=-1;
    srcfd=open(srcfilename,O_RDONLY);
    if(srcfd==-1)
    {
        perror("open error");
        exit(-1);
    }
    destfd=open(destfilename,O_WRONLY|O_CREAT,0644); //创建成功
    if(destfd==-1)
    {
        perror("open error");
        exit(-1);
    }

    close(destfd);

    //得到源文件的长度---是不是要用光标的那个函数
    int len=lseek(srcfd,0,SEEK_END);  //0表示保持不动
    close(srcfd);
    return len;
}

void copy(const char* srcfilename,const char* destfilename,int offset,int len) //两个外部文件
//光标起始位置(从起始位置开始的偏移量)，需要拷贝的长度 
{
    //先打开两个文件
    int srcfd=-1,destfd=-1;
    srcfd=open(srcfilename,O_RDONLY);
    if(srcfd==-1)
    {
        perror("open error");
        exit(-1);
    }
    destfd=open(destfilename,O_WRONLY);  
    if(destfd==-1)
    {
        perror("open error");
        exit(-1);
    }

    //分别移动两个文件的光标对齐
    lseek(srcfd,offset,SEEK_SET);
    lseek(destfd,offset,SEEK_SET);

    //先从源文件中读取数据到容器中，再把容器中的数据写入到目标文件中
    char buf[128];
    // read(srcfd,buf,len);
    // write(destfd,buf,len);

    //这么写太不严谨了吧，一次读取可能读取不完，应该循环着来
    while(len>0)
    {
                 //注：sizeof(buf)就是128，OS底层只会把buf当做字节流，不管C语言的字符串特点。只是个搬运工
       int res=min((int)sizeof(buf),len);  //本次读取的个数以及写入的个数，取最小值
       read(srcfd,buf,res);
       len-=res;
       write(destfd,buf,res);
    }

    close(srcfd);
    close(destfd);
}
int main(int argc, const char* argv[])
{
    //检查文件个数是不是三个
  if(argc!=3)
  {
    printf("input file error\n");
    printf("usage:./a.out srcfile destfile\n");
    return -1;
  }

  pid_t pid=-1;
  pid=fork();
  if(pid>0) //父进程
  {
     sleep(10); 
    copy(argv[1],argv[2],0,get_len(argv[1],argv[2])/2);
   
    wait(NULL);
  }
  else if(pid==0) //子进程
  {
    copy(argv[1],argv[2],get_len(argv[1],argv[2])/2,get_len(argv[1],argv[2])-get_len(argv[1],argv[2])/2);
  }
  else 
  {
     perror("fork error");
     return -1;
  }

  printf("拷贝成功\n");
  return 0;
}
