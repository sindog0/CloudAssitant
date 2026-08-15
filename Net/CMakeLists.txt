cmake_minimum_required(VERSION 3.10)
project(Net LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Net
    main.cpp
    Buffer.cpp
    TcpSocket.cpp
    EventLoop.cpp
    EpollTaskScheduler.cpp
    PacketReader.cpp
    PacketWriter.cpp
    TcpConnection.cpp
    Timer.cpp
    TaskScheduler.cpp
)

target_include_directories(Net PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
