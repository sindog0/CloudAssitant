
project(CloudNet LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)

add_library(cloud_net STATIC
    Acceptor.cpp
    BufferReader.cpp
    BufferWriter.cpp
    TcpSocket.cpp
    TcpServer.cpp
    EventLoop.cpp
    EpollTaskScheduler.cpp
    TcpConnection.cpp
    Timer.cpp
    TaskScheduler.cpp
)

target_include_directories(cloud_net
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(cloud_net
    PUBLIC
        Threads::Threads
)

target_compile_options(cloud_net
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)

add_executable(net_test
    main.cpp
)

target_link_libraries(net_test
    PRIVATE
        cloud_net
)