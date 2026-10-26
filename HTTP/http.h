#pragma once //防止头文件重复包含
#include<iostream>
#include<string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<stdio.h>
#include<sys/select.h>
#include<pthread.h>
#include<sys/un.h>
#include<fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#define SIZE 4096  //请求报文的最大字节数
extern char HTTP_IP[128];
extern int HTTP_PORT;   //只是声明，告诉编译器我在别处定义。不定义，不分配内存

int Init_Http(); //初始化服务器，并返回文件描述符

int handle_msg(int sockfd); //处理请求的入口  参数：用于通信的服务器文件描述符

int get_line(char** buf,int sockfd);
//作用：获取请求报文一行的数据，并返回获取的长度


void echo_error(int err_no,int sockfd); //错误码   输出对应的错误网页

int echo_www(int sockfd,char* path);  //输出对应网页
