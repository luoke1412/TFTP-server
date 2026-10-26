#include <stdlib.h>
#include<iostream>
#include<string.h>
#include<stdio.h>
using namespace std;
//测试字符串：以模块为单位输出到文件中，再以模块为单位输出到终端中
//关键：size_t n=fread(rbuf,1,sizeof(rbuf),fp);  读取字符串每一项的大小是1
void test01()
{
    //容器
    char wbuf[128];
    //从终端中写入数据到容器中
    fgets(wbuf,20,stdin); //第二个参数莫非是规定最大字节？
    wbuf[strlen(wbuf)-1]='\0'; //把换行符变为结束符

    //将容器中的数据以一个字符串为输出到文件中
    FILE* fp=NULL;
    fp=fopen("fopen","w");
   if(fp==NULL)
   {
        perror("fopen error");
        exit(EXIT_SUCCESS);
   }
    fwrite(wbuf,sizeof(wbuf),1,fp);

    fclose(fp);  //关闭并重新打开
    
     fp=fopen("fopen","r");
   if(fp==NULL)
   {
        perror("fopen error");
        exit(EXIT_SUCCESS);
   }
    //从文件中以模块为单位读取到容器中
    char rbuf[128];
    //size_t n=fread(rbuf,strlen(wbuf),1,fp);
    size_t n=fread(rbuf,1,sizeof(rbuf),fp);  //表示从文件中最多读取ruf项数据，每项一字节，存储到rbuf中
    
    rbuf[n]='\0';  //n为实际的项数，下标0~n-1，可以手动添加
    //输出到终端
    fputs(rbuf,stdout);
}

//读取整数--只写关键部分
void test02()
{
    fwrite(&num, sizeof(num), 1, fp); //将num中的数据写入fp指向的文件中

    //定义接收数据的变量
    int key = 0;
    //从文件中读取数据
    fread(&key, sizeof(key), 1, fp);
}

//读取类---只写关键部分
//假设定义了一个类class stu

void test03()
{
    //定义三个学生
    Stu s[3] = {{"张三",18, 98}, \
    {"李四",20, 88}, \
    {"王五",16, 95}};
    //将三个学生信息写入文件中
    fwrite(s, sizeof(Stu), 3, fp);


    //定义一个对象，接收读取的结果
    Stu temp;
    //从文件中读取第一个学生的信息
    fread(&temp, sizeof(Stu), 1, fp);
    //fread(&temp, sizeof(Stu), 1, fp);  第二个
    //fread(&temp, sizeof(Stu), 1, fp);  第三个
    //将读取的数据展示出来
    printf("name:%s, age:%d, score:%.2lf\n", temp.name, temp.age, temp.score);
}
int main(int argc, const char *argv[])
{
 test01();
 return 0;
}