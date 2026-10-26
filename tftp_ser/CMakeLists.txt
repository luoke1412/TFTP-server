cmake_minimum_required(VERSION 3.10)
project(tftp_ser)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(tftp_ser
    main.cpp
    tftp_ser.cpp
)
