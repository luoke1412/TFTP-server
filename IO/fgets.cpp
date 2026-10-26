//实验字符串的读入与读取
#include<iostream>
#include<stdio.h>
#include<string.h>
using namespace std;

//从stream指向的文件中最多读取size-1个字符到s容器中，遇到回车或文件结束，会结束一次读取
//先输出，再读取输出到终端
int test01()
{
    //打开文件
    FILE* fp=NULL;
    fp=fopen("./fopen","w");
    if(fp==NULL)
    {
        perror("fopen error");
        return -1;
    }

    //输入字符串
    char temp[50]="";
    while(1)
    {
        //每次都会从字符串的起始地址开始读入，会覆盖之前的值
        scanf("%s",temp); //cin
        //if(temp=="exit")
        if(strcmp(temp,"exit")==0)
        {
            break;
        }
        fputs(temp,fp);
        fputc('\n',fp); //换行
    }

    fclose(fp);

    //读取
    char rbuf[100]="";
    fp=fopen("./fopen","r");
    while(1)
    {
        //从文件中读取数据到rbuf容器中
        char* tag=fgets(rbuf,sizeof(temp),fp); //所以换行也是要被读取的是吗
        if(tag==NULL) break;
        printf("%s\n",rbuf);
    }
    //关闭文件
    fclose(fp);
    fp=NULL;
}
int main(int argc, const char *argv[])
{
  test01();
  return 0;
}