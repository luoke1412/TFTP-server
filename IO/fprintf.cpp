//演示fprintf和fscanf
#include<iostream>
#include<stdio.h>
#include<cstdlib>
#include<string.h>
using namespace std;

//向终端文件输出内容，并从终端中读取内容
void test01() 
{
    int a=4;
    fprintf(stdout,"你知道%d吗\n",a);
    fscanf(stdin,"%d",&a);
    fprintf(stdout,"%d呀\n",a);
}

//向外部文件输出内容，并读取内容
int test02()
{
    FILE* fp=NULL;
    fp=fopen("fopen","r+");
    if(fp==NULL)
    {
        perror("fopen error");
        return -1;
    }
    char Admin[20]="";
    int age;
    fscanf(fp,"%s %d",Admin,&age); //从文件中读数据(提前准备好) 和scanf一样，遇到空格结束

    fprintf(stdout,"管理员是%s，年龄是%d\n",Admin,age); //打印在终端

     //fscanf(stdin,"手下是%s，年龄是%d",Admin,&age); //从终端输入

    fscanf(stdin,"%s %d",Admin,&age); //从终端输入
     fprintf(stdout,"管理员是%s，年龄是%d\n",Admin,age); //打印在文件中
}

/*
练习:使用fprintf和fscanf完成注册和登录功能，要求做个小菜单，三个功能，功能1是注册，用户输入账号和
密码后，将其存放到外部文件中；功能2是登录，用户输入登录账号和密码后，如果跟文件中匹配，则
提示登陆成功，如果全部不匹配，则提示登录失败；功能0，表示退出。菜单可以循环调用
*/

bool login()  //登录。和数据库中所有人的信息核对
{
    char Admin[128];
    char Pass[128];
    int Age;
    FILE* fp=NULL;
    fp=fopen("./database","r");
    if(fp==NULL)
    {
        perror("fopen error:");
        return 0;
    }
    //从终端读取数据
    fprintf(stdout,"用户名是：");
    fscanf(stdin,"%s",Admin);
    fprintf(stdout,"密码是：");
    fscanf(stdin,"%s",Pass);
    fprintf(stdout,"年龄是：");
    fscanf(stdin,"%d",&Age);

    //从文件中读取数据并进行比对
    char rAdmin[128]; //已经登录过的数据
    char rPass[128];
    int rAge;
    while(1)
    {
        if(fscanf(fp,"%s",rAdmin)==EOF) //能走到这里说明没数据了
        {
            return 0;
        }
        fscanf(fp,"%s",rPass);
        fscanf(fp,"%d",&rAge);
        
        //return Admin==rAdmin&&rPass==Pass&&rAge==Age;
        if(strcmp(Admin,rAdmin)==0&&strcmp(Pass,rPass)==0&&rAge==Age) return 1;
    }
    
    fclose(fp);
    
}

bool register_() //注册。并且可以存储很多人的注册信息(数据库)
{
    char Admin[128];
    char Pass[128];
    int Age;
    FILE* fp=NULL;
    fp=fopen("./database","a+");
    if(fp==NULL)
    {
        perror("fopen error:");
        return 0;
    }
    //输出提示在终端，从终端读取信息， 输出(保存)信息在文件
    fprintf(stdout,"用户名是：");
    fscanf(stdin,"%s",Admin);
    fprintf(fp,"%s ",Admin);

    fprintf(stdout,"密码是：");
    fscanf(stdin,"%s",Pass);
    fprintf(fp,"%s ",Pass);

    fprintf(stdout,"年龄是：");
    fscanf(stdin,"%d",&Age);
    fprintf(fp,"%d",Age);

    //来个换行
    fprintf(fp,"\n");
    fclose(fp);  //关闭文件，同时保存
    return 1;
}
int main(int argc, const char *argv[])
{
    //test02();
    int opt;
    while(1)
    {
        fprintf(stdout,"0:退出\n");
        fprintf(stdout,"1:注册\n");
        fprintf(stdout,"2:登录\n");
        fprintf(stdout,"请输出你的选择:");
        fscanf(stdin,"%d",&opt);
        getchar();  //吸收缓冲区的换行符，避免下一个输入的是字符时会出错
        if(opt==0)
        {
            cout<<"退出成功~\n";
            return 0;
        }
        else if(opt==1)
        {
            system("clear");
            register_();
            cout<<"注册成功~\n";
        }
        else if(opt==2)
        {
             system("clear");
            if(login()) cout<<"登录成功~\n";
            else cout<<"登录失败\n";
        }
    }

    return 0;
}