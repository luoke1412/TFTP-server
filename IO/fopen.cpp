//实验文件的开关方式与错误码的打印
/*1.打开方式的特点： 如果可读r，那么光标一定是从开头开始。如果只是r是打不开不存在的文件的
     如果可写w-->不存在就创建，存在则会全部清空               a:往后添加
*/
#include<iostream>
#include<stdio.h>
using namespace std;

//测试1：无法以r/r+的方式打开不存在的文件
int test01()
{
    //1.定义一个文件指针
    FILE* fp=NULL;
    fp=fopen("./fopen","r+");
    if(fp==NULL)
    {
        perror("fopen error");
        return -1;
    }
    printf("文件打开~");
    fclose(fp);
    fp=NULL;
    return 0;
}

//测试2：没有文件，w/w+可以新建文件
int test02()
{
    //1.定义一个文件指针
    FILE* fp=NULL;
    fp=fopen("./fopen","w");
    if(fp==NULL)
    {
        perror("fopen error");
        return -1;
    }
    printf("文件打开~\n");
    fclose(fp);
    fp=NULL;
    return 0;
}

//测试3：以w/w+的方式打开已存在的文件，会清空所有内容。a是直接添加
int test03()
{
    //1.定义一个文件指针
    FILE* fp=NULL;
    fp=fopen("./havecontent.md","w+");
    if(fp==NULL)
    {
        perror("fopen error");
        return -1;
    }
    printf("文件打开~\n");
    fclose(fp);
    fp=NULL;
    return 0;
}
int main(int argc, const char *argv[])
{
    test03();
    return 0;
}