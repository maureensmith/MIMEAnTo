cmake_minimum_required(VERSION 3.5)
project(MIMEAnTo_console)

set(CMAKE_CXX_STANDARD 20)

#SET(CMAKE_BUILD_TYPE Debug)

INCLUDE_DIRECTORIES(.)


add_executable(MIMEAnTo
        main.cpp
        ioTools.cpp
        ioTools.hpp
        mimeexception.cpp
        mimeexception.hpp
        processing.cpp
        processing.hpp
        utils.cpp
        utils.hpp)

#for g++-9 this is not necessary, but it is for g++-8
target_link_libraries(MIMEAnTo stdc++fs)