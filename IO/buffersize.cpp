//测试两个东西，一个是缓冲区的大小，一个是缓冲区的晒新时机.
//这里测试缓冲区大小
#include<iostream>
#include<stdio.h>
#include <stdlib.h>
using namespace std;

void test02() //全缓存
{
    FILE* fp=NULL;
    fp=fopen("./fopen","w");
    if(fp==NULL)
    {
        perror("fopen error");
        exit(1);
    }
    fputs("hello",fp);
    printf("size=%d\n",fp->_IO_buf_end -fp->_IO_buf_base);

}

int main(int argc, const char *argv[])
{
    /*     行缓存
    //如果缓冲区没有被使用时，求出大小为0，只有被至少使用了一次后，缓冲区的大小就被分配了
    fputc('A',stdout);  //输出内容到终端
    
    printf("size=%d\n",stdout->_IO_buf_end - stdout->_IO_buf_base);
    
    char m=fgetc(stdin); //读取终端的一个字符并返回
    printf("%c",m);
    printf("size=%d\n",stdin->_IO_buf_end - stdin->_IO_buf_base);

    */
    //不缓存
    printf("size=%d\n",stderr->_IO_buf_end - stderr->_IO_buf_base);
    
    test02();

    return 0;
}

