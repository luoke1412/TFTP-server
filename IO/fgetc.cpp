//实验单字符的读取与读入
//注意：put和get是相对于原程序来说的，输入到文件是put，从文件中读入是get
#include<iostream>
#include<stdio.h>
using namespace std;

int test01() //试一试,输入到文件后在读入到内存当中
{
    //打开/创建文件
    FILE* fp=NULL;
    fp=fopen("./fopen","w");
    if(fp==NULL)
    {
        perror("fopen error");
        return -1;
    }

    //写入字符： fputc
    fputc('h',fp);
     fputc('e',fp);
      fputc('l',fp);
       fputc('l',fp);
        fputc('o',fp);

    //读入字符：fgetc。
    //那么现在可以直接读吗？  不能。一是因为现在光标是在末尾，读不了。二是现在是只写
    //方法一：改成w+，光标移到开头    方法二：关闭文件，重新开r
    fclose(fp);


    fp=fopen("./fopen","r");
    if(fp==NULL)
    {
        perror("fopen error");
        return -1;
    }

    char temp;
    while(1)
    {
        temp=fgetc(fp);
        if(temp==EOF)
        {
            break;
        }
        printf("%c",temp);
    }
    return 0;
}


//使用fgetc和fputc完成两个文件的拷贝工作，实现指令cp的功能： cp srcfile destfile
/*
拆分步骤： 1.先打开两个文件
          2.将sr文件的内容读入到内存中(可读)
          3.将内存中的数据输出到内存中(可写)
*/
int test02()
{
    //1.
    FILE* src=NULL;
    FILE* dest=NULL;
    src=fopen("./srcfile","r");
    dest=fopen("./destfile","w");

     if(src==NULL||dest==NULL)
    {
        perror("fopen error");
        return -1;
    }

    //2.
    //char buf[100]="";
    while(1)
    {
        int temp=fgetc(src);
        if(temp==EOF)
        {
            break;
        }
        fputc((char)temp,dest);  
    }
    fclose(dest);
    fclose(src);
    return 0;
}

int main(int argc, const char *argv[])
{
    //0.判断外部传参的个数是否为 3,3个文件一起执行
    if(argc != 3)
    {
    printf("input file error\n");
    printf("usage:./a.out srcfile destfile\n");
    return -1;
    }
    test02();
    return 0;
}