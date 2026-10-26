# 版本
cmake_minimum_required(VERSION 3.20)
#项目名称
project(TFTP_CLI)

#设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED on)

#添加可执行程序
add_executable(cli src/main.cpp src/tftp_cli.cpp)

# #添加头文件路径
# include_directories(./include)

# 现代写法：把头文件路径精准绑定到 cli 这个目标上（PUBLIC 表示如果别人链接 cli，也能用到这个路径）
target_include_directories(cli PUBLIC ${PROJECT_SOURCE_DIR}/include)

# 因为之前编译失败过，必须清空 build目录的旧缓存
# rm -rf *