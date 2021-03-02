cmake_minimum_required(VERSION 3.5)
project(MIMEAnTo_console)
#set(Boost_USE_STATIC_LIBS OFF)
#set(Boost_USE_MULTITHREADED OFF)
#set(Boost_USE_STATIC_RUNTIME OFF)

#SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "/Users/msmith/homebrew/Cellar/boost/1.75.0_2/include")

#find_package(Boost REQUIRED COMPONENTS system filesystem iostreams)
#MESSAGE(FATAL_ERROR "${Boost_LIBRARIES}")
set(CMAKE_CXX_STANDARD 20)
#SET(CMAKE_OSX_SYSROOT "")
#SET(CMAKE_BUILD_TYPE Debug)

INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(gnuplot-iostream ${Boost_INCLUDE_DIR})


add_executable(MIMEAnTo
        main_c.cpp
        ioTools.cpp
        ioTools.hpp
        #plot.cpp
        #plot.hpp
        mimeexception.cpp
        mimeexception.hpp
        processing.cpp
        processing.hpp
        utils.cpp
        utils.hpp)

#target_link_libraries(MIMEAnTo ${Boost_LIBRARIES})

#for g++-9 this is not necessary, but it is for g++-8
target_link_libraries(MIMEAnTo stdc++fs)