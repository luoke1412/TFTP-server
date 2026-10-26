# 最小支持版本
cmake_minimum_required(VERSION 3.20)
# 项目名称
project(HELLO CXX)



#生成可执行程序， 依赖hello.cpp生成hello可执行程序
add_executable(hello_cmake hello.cpp)